# RCompiler

A Rust-subset (`.rx`) to RISC-V 64-bit compiler written in C++20. The compiler generates RV64 assembly and runs via `qemu-riscv64`.

## Build

```bash
# Configure + build all targets into build/
make build

# Or manually:
mkdir -p build && cd build && cmake .. && cmake --build .
```

The CMake build produces 9 targets: `parser_test`, `semantic_test`, `IR_test`, `codegen_test`, `reg_alloc_test`, `IR_single`, `IR_single_optimized`, `codegen_single`, `dominator`.

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
  → mem2reg → EliminateCriticalEdge → ReplacePhiWithMoves
  → EliminateEmptyBlocks → Preprocessor → Memory Allocator → Reg Alloc
  → Assembly Generator → RISC-V .s
```

## Architecture

### Source layout (under `src/`)

- **`lexer/`** — Tokenizer: tries each token function at each position, picks the longest match.
- **`parser/`** — Recursive-descent parser with Pratt parsing for operator precedence.
- **`semantic/`** — Scope tree, type system with `ConstValue`, builtin types/functions.
- **`visitor/`** — AST visitors: three checker passes, IR generator, AST pretty-printer.
- **`IR/`** — Three-address-code IR: functions contain blocks, blocks contain instructions. Values are ptr-based.
- **`IR_visitor/`** — IR passes: preprocessor, memory allocator, assembly generator, phi eliminator, empty block eliminator.
- **`liveness_analysis/`** — CFG construction, def-use sets, liveness in/out (worklist algorithm), dominator tree.
- **`mem2reg/`** — Memory-to-register promotion (alloca→SSA registers with phi insertion) and critical edge elimination.
- **`reg_alloc/`** — Graph-coloring register allocator with Briggs conservative move coalescing.
- **`codegen/`** — RISC-V assembly emission helpers, register name table, phi topological ordering.

### IR naming conventions

- Function `foo` → `function..foo`
- Struct `s` → `struct.s`
- Method `s.foo` → `function..s.foo`

## RISC-V register conventions

### t-reg usage (codegen scratch)

| register | purpose |
|---|---|
| **t0** | arith (operand1), compare aux (sub, sltu, slt), data move, call args |
| **t1** | arith (operand2), store const, GEPP (slli, li), saved/restored around calls |
| **t2** | arith result (kMemory), GEPP (mul) |
| **t3** | **constant cache** — pre-loaded at function entry with a large (>12-bit) constant; reloaded via `li` after calls |
| **t4** | **constant cache** — second pre-loaded constant slot |
| **t5** | long jumps for large functions (`lui+addi+jalr`); used for promoted variables in leaf functions |
| **t6** | `PrintIA`/`PrintIStar`/`PrintMem` fallback for out-of-range immediates |

### Reg alloc color pool

- **Precolored nodes** (function params): a0–a7 (IDs 10–17)
- **Leaf functions**: t5 → unused a-regs → s1–s11 → parameter a-regs (no s-reg save/restore needed)
- **Non-leaf functions**: s1–s11 (IDs 9, 18–27) → a0–a7 as fallback
- Briggs conservative move coalescing merges move-related virtual registers that don't interfere

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

- `s_save = 8 * |used_s_regs|`, `total_stack = stack_size_ + s_save`
- `total_stack` is rounded up to the nearest multiple of 16 for RISC-V ABI compliance
- Leaf functions (no calls) skip the entire a-reg/ra save area
- Memory addresses assigned after reg_alloc: only `kMemory` variables get stack slots
- t3/t4 constants reloaded via `li` after calls, not saved to memory

## Key optimizations

### Codegen

- **Immediate folding**: Small constants (12-bit signed) in `+`/`-` folded into `addiw`
- **Redundant jump elimination**: Unconditional jumps to the next block elided; branch false-targets fall through
- **Constant hoisting**: Up to 2 large constants pre-loaded into t3/t4 at function entry
- **Compare peephole**: `==0`/`!=0` use `sltiu`/`sltu` with immediate 1 or x0 (no `li t1, 0`)
- **Long jumps**: Functions with >40k instructions or >800 blocks use `lui+addi+jalr` (±2GB range)
- **Call-save consolidation**: `RestoreRegister`+`SaveRegister` skipped between consecutive calls

### IR

- **RVO and direct-write**: Struct/array literals write directly into the return buffer or let target, skipping temp allocation and `memcpy`
- **BranchContext**: Comparison/lazy-boolean results branch directly instead of storing to memory
- **Leaf function stack**: Skip a-reg/ra save area when no calls and all params fit in registers
- **Leaf function reg-alloc**: Prefer caller-saved registers (t5, a-regs) over s-regs, eliminating prologue/epilogue save/restore

### Liveness & mem2reg

- **Worklist algorithm**: CalcInOut only re-queues predecessors when a block's IN set changes
- **Batched SSA rename**: Single dominator tree walk processes all promoted variables simultaneously
- **Phi-merge elimination**: Blocks containing only a branch on a phi variable are threaded into predecessors, collapsing patterns from `&&`/`||` chains

## RISC-V 64 toolchain

The RV64 linker and cross-compilation libraries are stored under `tools/riscv64-linux/`:
- `lld/bin/` — extracted `lld-21` package (LLVM linker with RISC-V support)
- `sysroot/` — extracted cross-compilation libraries

Source `.deb` files are in `tools/debs/`. To set up on a new machine, extract them with `dpkg-deb -x` into `tools/riscv64-linux/`.

## Performance tracking

Cycle counts from the RISC-V simulator across compiler versions are tracked in `performance/`. Run `python3 performance/visualize.py` to generate comparison plots.

## Testcases submodule

Test cases are in the `testcases/` submodule (`github.com/peterzheng98/RCompiler-Testcases`). After cloning, run `git submodule update --init`.
