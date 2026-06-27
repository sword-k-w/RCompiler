# RCompiler

A Rust-subset (`.rx`) to RISC-V 64-bit compiler written in C++20. The compiler generates RV64 assembly and runs via `qemu-riscv64`.

## Build

```bash
# Configure + build all targets into build/
make build

# Or manually:
mkdir -p build && cd build && cmake .. && cmake --build .
```

CMakeLists.txt currently enables `code` (production driver, links `test/codegen_submit.cpp`) and `codegen_single` (developer driver). Other test targets exist in the file but are commented out — uncomment to build.

## Test

```bash
# All CTest-registered tests
cd build && ctest

# Run a specific GTest suite/case by filter
./build/codegen_test --gtest_filter=TestSuiteName.TestName

# Run tests in isolation with XML reports (Python runner)
cd test && python3 run_isolated_tests.py
```

Test cases live in `testcases/` (git submodule). Scripts under `script/IR/` and `script/codegen/` drive end-to-end testing through `qemu-riscv64`. The test scripts require `clang-21`, `lld-21`, and RISC-V Linux cross-compilation libraries (extracted under `tools/riscv64-linux/`).

## Pipeline

```
Source (.rx) → Lexer → Parser → Semantic (3 passes) → IR Generator
  → FunctionInliner → mem2reg → EliminateCriticalEdge → SCCP → CSE
  → ReplacePhiWithMoves → EliminateEmptyBlocks
  → Preprocessor → MemoryAllocator → ParameterDemoter → RegAlloc
  → ParameterDemoter::FixupAfterRegAlloc → AssemblyGenerator → RISC-V .s
```

The driver lives in `test/codegen_submit.cpp`.

## Architecture

### Source layout (under `src/`)

- **`lexer/`** — Tokenizer: tries each token function at each position, picks the longest match.
- **`parser/`** — Recursive-descent parser with Pratt parsing for operator precedence.
- **`semantic/`** — Scope tree, type system with `ConstValue`, builtin types/functions.
- **`visitor/`** — AST visitors: three checker passes, IR generator, AST pretty-printer.
- **`IR/`** — Three-address-code IR: functions contain blocks, blocks contain instructions. Values are ptr-based.
- **`IR_visitor/`** — IR passes:
  - `function_inliner/` — bottom-up inliner for callees ≤50 IR instructions, non-recursive.
  - `sccp/` — Sparse Conditional Constant Propagation (lattice + executable-edge tracking).
  - `cse/` — within-block CSE for GEP/GEPP.
  - `phi_eliminator/`, `empty_block_eliminator/` — post-SSA cleanup.
  - `preprocessor/`, `memory_allocator/` — type sizing + stack layout.
  - `parameter_demoter/` — demote a-reg params via `mv` so RegAlloc can recolor them.
  - `assembly_generator/` — IR→RISC-V assembly, the final backend.
- **`liveness_analysis/`** — CFG construction, def-use sets, liveness in/out (worklist algorithm), dominator tree.
- **`mem2reg/`** — Memory-to-register promotion (alloca→SSA registers with phi insertion) and critical edge elimination.
- **`reg_alloc/`** — Graph-coloring register allocator with Briggs conservative move coalescing; records per-call-site liveness for dead-a-reg masks.
- **`codegen/`** — RISC-V assembly emission helpers, register name table, phi topological ordering.

### IR naming conventions

- Function `foo` → `function..foo`
- Struct `s` → `struct.s`
- Method `s.foo` → `function..s.foo`

## IR optimization passes

- **Function inliner**: clones non-recursive callees of ≤50 IR instructions at each call site, re-IDing blocks and remapping names via a per-caller `NameAllocator`. Runs to fixpoint.
- **SCCP**: lattice cells `Top`/`Constant(i32)`/`Bottom`; the executable-edges set prevents dead branches from poisoning the lattice. Folds constants, retargets dead branches into unconditional jumps, drops phi inputs on unexecutable edges.
- **CSE (within block, GEP/GEPP only)**: dedupes address computations with the same `(base, [index], type-key)`. `EnsureTypeSize()` pre-computes the dropped GEP's `allocated_size_`/`align_` so downstream passes still see them.
- **Parameter demotion** (Phase 3): before RegAlloc, prepend `%x.mv ← %x` moves for every a-reg parameter and rename all uses to `%x.mv`. RegAlloc then colors `%x.mv` like any other virtual — typically into an s-reg, t5, or a free a-reg. `FixupAfterRegAlloc` writes back the assigned stack slot into the demotion move's address.

## RISC-V register conventions

### t-reg usage (codegen scratch)

| register | purpose |
|---|---|
| **t0** | arith (operand1), compare aux (sub, sltu, slt), data move, call args |
| **t1** | arith (operand2), store const, GEPP (slli, li), saved/restored around calls |
| **t2** | arith result (kMemory), GEPP (mul) |
| **t3** | **constant cache** — pre-loaded at function entry with a large (>12-bit) constant; reloaded via `li` after calls |
| **t4** | **constant cache** — second pre-loaded constant slot |
| **t5** | long jumps for large functions (`lui+addi+jalr`); also in the reg-alloc pool — used for any leaf-function virtual, or for any non-leaf virtual whose live range doesn't cross a call |
| **t6** | `PrintIA`/`PrintIStar`/`PrintMem` fallback for out-of-range immediates |

### Reg alloc color pool

- **Precolored** (function params): a0–a7 (IDs 10–17).
- **Leaf functions**: t5 → unused a-regs → s1–s11 → parameter a-regs (no s-reg save/restore needed).
- **Non-leaf functions**: s1–s11 → a0–a7 → t5. The interference graph strips t5 edges from variables that don't cross a call (`call_crossing_vars_`), so t5 is only assigned to non-call-crossing values.
- **Per-call dead-a-reg masks**: RegAlloc snapshots live sets at every call site and stores `dead_a_regs_mask_` per call; `SaveRegister`/`FlushSavedRegisters` skip the `sd`/`ld` pair for a-regs that hold no live value at the call.
- Briggs conservative move coalescing merges move-related virtual registers that don't interfere. A pre-pass removes spurious "src-killed-when-dst-defined" interference edges so phi-move coalescing inside loops actually fires.

## Stack frame layout

s-regs at the bottom (sp), a-reg/ra at the top (within 72 bytes). The two areas never overlap.

```
High addresses (sp + total_stack):
  ┌──────────────────────────────┐
  │ a0 save             (off  8) │ ┐
  │ a1 save             (off 16) │ │ SaveRegister (packed tightly,
  │ ...                          │ │ no t-reg space — t-regs are
  │ a7 save             (off 64) │ │ caller-saved scratch, dead
  │ ra save             (off 72) │ ┘ after each instruction)
  ├──────────────────────────────┤ ← top of stack_size_
  │ local variables & spills     │ ← assigned after reg_alloc
  ├──────────────────────────────┤ ← sp + s_save
  │ s1 save             (off  0) │ ┐ prologue: s-regs at
  │ s2 save             (off  8) │ │ sp + 0, 8, 16, ...
  │ ...                          │ ┘
  └──────────────────────────────┘
Low addresses (sp):
```

- `s_save = 8 * |used_s_regs|`, `total_stack = stack_size_ + s_save`.
- `total_stack` rounded up to a multiple of 16 for RISC-V ABI compliance.
- Leaf functions (no calls) skip the entire a-reg/ra save area.
- Memory addresses assigned after reg_alloc: only `kMemory` variables get stack slots.
- `i32` slot size is 4 bytes (not 8).
- t3/t4 constants reloaded via `li` after calls, not saved to memory.

## Key optimizations

### Codegen

- **Immediate folding**: small constants (12-bit signed) in `+`/`-` folded into `addiw`.
- **Redundant jump elimination**: unconditional jumps to the next block elided; branch false-targets fall through.
- **Block layout**: DFS reorder from entry; branches prefer the false target as next so `bnez`/`beqz` falls through the common path.
- **Constant hoisting**: up to 2 large constants pre-loaded into t3/t4 at function entry. Scan covers IR operands AND kMemory variable stack offsets AND a-reg/ra save-slot offsets (weighted by call count). Cached immediates avoid `li t6` at every load/store.
- **Compare peephole**: `==0`/`!=0` use `sltiu`/`sltu` with immediate 1 or x0 (no `li t1, 0`).
- **Compare→branch fusion**: a single-use `kMemory` compare immediately before a branch parks its t-reg result in `fused_cmp_branch_reg_`; the branch reads it directly, skipping the `sd`/`ld` pair.
- **Long jumps**: functions with `est_asm > 8000` or `blocks > 500` use `lui+addi+jalr` (±2GB range).
- **Call-save consolidation**: `RestoreRegister`+`SaveRegister` skipped between consecutive calls — flushed only at block exits.
- **Per-call dead a-reg masks**: every call site carries a bitmask of a-regs known dead; saves and reloads for those a-regs are skipped.

### IR

- **RVO and direct-write**: struct/array literals write directly into the return buffer or let target, skipping temp allocation and `memcpy`.
- **BranchContext**: comparison/lazy-boolean results branch directly instead of storing to memory.
- **Loop-form array init**: `[val; N]` initializations emit a loop instead of unrolling N stores.
- **Inline small struct copy**: small struct copies are lowered to `ld`/`sd` pairs instead of a `builtin_memcpy` call.
- **Parameter demotion**: a-reg parameters are renamed to virtuals at the entry block so the colorer chooses their physical register.
- **Leaf function stack**: skip the a-reg/ra save area when no calls and all params fit in registers.
- **Leaf function reg-alloc**: prefer caller-saved registers (t5, a-regs) over s-regs, eliminating prologue/epilogue save/restore.

### Liveness & mem2reg

- **Worklist algorithm**: CalcInOut only re-queues predecessors when a block's IN set changes.
- **Batched SSA rename**: single dominator tree walk processes all promoted variables simultaneously.
- **Phi-merge elimination**: blocks containing only a branch on a phi variable are threaded into predecessors, collapsing patterns from `&&`/`||` chains.

## RISC-V 64 toolchain

The RV64 linker and cross-compilation libraries are stored under `tools/riscv64-linux/`:
- `lld/bin/` — extracted `lld-21` package (LLVM linker with RISC-V support).
- `sysroot/` — extracted cross-compilation libraries.

Source `.deb` files are in `tools/debs/`. To set up on a new machine, extract them with `dpkg-deb -x` into `tools/riscv64-linux/`.

## Performance tracking

Cycle counts from the RISC-V simulator across compiler versions are tracked in `performance/`. Run `python3 performance/visualize.py` to generate comparison plots.

## Testcases submodule

Test cases are in the `testcases/` submodule (`github.com/peterzheng98/RCompiler-Testcases`). After cloning, run `git submodule update --init`.
