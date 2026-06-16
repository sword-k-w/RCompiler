# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

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

CI (`.github/workflows/cmake-ci.yml`) triggers on push to `main`/`master`, builds all targets and runs `ctest` on `ubuntu-latest`.

Test cases live in `testcases/` (git submodule). Scripts under `script/IR/` and `script/codegen/` drive end-to-end testing through `qemu-riscv64`. The test scripts require `clang-21`, `lld-21`, and RISC-V Linux cross-compilation libraries (extracted under `tools/riscv64-linux/`).

## Architecture

### Compiler pipeline

```
Source (.rx) → Lexer → Parser → Semantic (3 passes) → IR Generator
  → mem2reg → EliminateCriticalEdge → ReplacePhiWithMoves
  → EliminateEmptyBlocks → Preprocessor → Memory Allocator → Reg Alloc
  → Assembly Generator → RISC-V .s
```

### Source layout (under `src/`)

- **`lexer/`** — Tokenizer: tries each token function at each position, picks the longest match. Single-char, multi-char, literal, and whitespace/comment tokens are split across separate files.
- **`parser/`** — Recursive-descent parser. Expression parsing uses Pratt parsing for operator precedence. AST nodes each live in `parser/node/`. Template method `Parser::Run<T>()` instantiates parsing for a specific AST node type. In `ExpressionStatementNode`, `ExpressionWithBlockNode` is tried first (before `ExpressionWithoutBlockNode`) to avoid exponential backtracking on deeply nested `if`/`else` chains.
- **`semantic/`** — Scope tree (`scope/`) with separate type/value namespaces. Type system (`type/`) with `ConstValue` for compile-time evaluation. Builtin types/functions defined in `builtin/`.
- **`visitor/`** — AST visitors via `VisitorBase` (~50 Visit methods). Three checker passes: first (scope building, name collection, impl binding), second (type inference, const evaluation), third (type checking). Also `IR_generator/` (AST→IR) and `printer/` (AST pretty-printer).
- **`IR/`** — Three-address-code IR: functions contain blocks, blocks contain instructions. Values are ptr-based (each variable/expression gets a named pointer). Key instruction types: Arithmetic, Branch, Jump, Return, Allocate, Load, Store, GEP, Compare, Call, Phi, Select.
- **`IR_visitor/`** — IR passes via `IRVisitorBase`: `preprocessor/`, `memory_allocator/` (stack frame layout), `assembly_generator/` (IR→RISC-V assembly, the final backend).
- **`liveness_analysis/`** — CFG construction, def-use sets, liveness in/out, dominator tree (for SSA construction). `CalcInOut()` uses a worklist algorithm: when a block's IN set changes, only its predecessors are re-queued (since `OUT[pred] = ∪ IN[succ]`). This avoids full fixed-point scans over all blocks, reducing O(n²) to near-linear for deeply nested control flow.
- **`mem2reg/`** — Memory-to-register promotion (alloca→SSA registers with phi insertion) and critical edge elimination.
- **`IR_visitor/phi_eliminator/`** — Replaces phi instructions with explicit move instructions in predecessor blocks.
- **`IR_visitor/empty_block_eliminator/`** — Eliminates empty pass-through blocks after phi elimination, including phi-merge blocks (blocks containing only a branch on a phi variable, created by `&&`/`||` chains). Threads the branch into jump-predecessors and removes the merge block.
- **`reg_alloc/`** — Graph-coloring register allocator. Builds interference graph from per-instruction liveness (reverse block walk), promotes scalar variables (i32/i1/ptr) into registers. Uses Chaitin-style simplify/spill/select with worklist. Added Briggs conservative move coalescing: move-related virtual registers are merged into one node if they don't interfere and the merged degree < K, eliminating `mv` instructions.
- **`codegen/`** — RISC-V assembly emission helpers, register name table (32 regs), phi topological ordering.
- **`common/`** — Keywords, builtin definitions, error reporting, helpers.
- **`data_loader/`** — File/stdin input.

All headers live under `src/include/`, mirroring the source tree structure.

### IR naming conventions

- Function `foo` → `function..foo`
- Struct `s` → `struct.s`
- Method `s.foo` → `function..s.foo`

### Key design patterns

- **Visitor pattern**: Both the AST and IR use abstract visitor bases. All compiler passes (semantic checks, IR generation, IR optimization, codegen) are visitors.
- **Pratt parsing**: Expression parsing uses Pratt's algorithm for operator precedence.
- **Template-based parser dispatch**: `Parser::Run<T>()` templates generate parsing code for each AST node type.
- **Ptr-based IR values**: Each variable/expression stores its value via a named pointer; left-values use their own pointer, right-values use a temporary pointer.
- **RVO and direct-write optimization**: The IR generator avoids temporary allocations and `memcpy` for struct/array literals via three cooperating fields:
  - `rvo_target_` — set to `%pass..pointer` when the current function has a struct/array return type (the caller passes a return buffer pointer). In `Return()`, if the expression's IR name already equals `rvo_target_`, the `memcpy` is skipped entirely.
  - `in_return_` — flag set while visiting a return expression. When both `rvo_target_` and `in_return_` are set, `StructExpressionNode`/`ArrayExpressionNode` use the pass pointer as their base instead of allocating locally.
  - `let_target_` — set to the pattern's IR name when a `let` statement's RHS is a struct/array literal. The literal writes directly into the let variable, eliminating the temp+memcpy.
  - **Nested propagation**: When a struct/array field is itself a struct/array literal, the parent pre-computes a GEP into the target and sets it as `let_target_` for the child, propagating the optimization recursively through nested literals.

### RISC-V temp register conventions (codegen)

- **t0**: arith (operand1), compare aux (sub, sltu, slt), data move, call args
- **t1**: arith (operand2), store const, GEPP (slli, li), saved/restored around calls
- **t2**: arith result (kMemory), GEPP (mul)
- **t3**: **constant cache** — pre-loaded at function entry with a large (>12-bit) constant.  Survives calls via `li` reload in RestoreRegister.  (Formerly compare aux; merged into t0.)
- **t4**: **constant cache** — second pre-loaded constant slot.
- **t5**: long jumps for large functions (`lui+addi+jalr`); also used for promoted variables in leaf functions (safe since leaf functions don't need long jumps)
- **t6**: `PrintIA`/`PrintIStar`/`PrintMem` fallback for out-of-range immediates

Constants are pre-scanned per-function and up to 2 large constants are
loaded into t3/t4 at function entry.  `VariableToReg` checks `const_cache_`
and returns the cached register instead of emitting inline `li`.  After
each `call`, `RestoreRegister` re-emits `li` to reload the constants
(t-regs are caller-saved).  Functions without large constants have zero
overhead.

### Phi handling

Phis in the same block are calculated in parallel (see `codegen/phi_topo.cpp`).

### Stack frame layout (codegen)

s-reg saves live at the bottom of the frame (near sp), a-reg/ra saves at the top (within a 72-byte reserved area). The two areas never overlap.

```
High addresses (sp + total_stack):
  ┌──────────────────────────────┐
  │ a0 save             (off  8) │ ┐
  │ a1 save             (off 16) │ │ SaveRegister / RestoreRegister
  │ ...                          │ │ packed tightly: no t-reg space
  │ a7 save             (off 64) │ │ t-regs are caller-saved scratch,
  │ ra save             (off 72) │ ┘   dead after each instruction
  ├──────────────────────────────┤ ← top of original stack_size_
  │                              │
  │ local variables & spills     │ ← assigned after reg_alloc
  │                              │
  ├──────────────────────────────┤ ← bottom of variable area (sp + s_save)
  │ s1 save             (off  0) │ ┐ prologue / epilogue: s-regs
  │ s2 save             (off  8) │ │ packed at sp + 0, 8, 16, ...
  │ ...                          │ ┘
  └──────────────────────────────┘
Low addresses (sp):
```

- `s_save = 8 * |used_s_regs|`, `total_stack = stack_size_ + s_save`
- `total_stack` is rounded up to the nearest multiple of 16 for RISC-V ABI compliance.
- s-regs at `sp + 0, sp + 8, ...` (bottom of frame)
- a-regs at `current_stack_ - 8 - 8*i`, ra at `current_stack_ - 8 - 8*current_a_reg_used_` (top of frame)
- t-regs are pure scratch (caller-saved, dead after each instruction), no save space reserved
- t3/t4 constants are reloaded via `li` after calls, not saved to memory
- Variable access: `sp + current_stack_ - address`

### Memory allocation

MemoryAllocator records sizes without assigning addresses.  After reg_alloc promotes scalar variables to registers, only variables still in memory (`kMemory`) receive contiguous stack addresses.  Promoted variables never occupy stack space, so no compaction is needed.

### Leaf function optimizations

Two cooperating optimizations eliminate unnecessary stack frame overhead for leaf functions (functions with no calls):

- **Skip a-reg/ra save area** (IR / MemoryAllocator): Leaf functions whose parameters all fit in a0–a7 don't need the 72-byte caller-save area.  Previously every function allocated this area, causing simple leaf functions to waste ~144 bytes of stack with s-reg save/restore.  After this change, leaf function stack frames shrink to ~16 bytes (just the s-reg save area, if any s-regs are used).  Struct/array Load/Store/Move instructions that lower to `builtin_memcpy` are also detected as calls.

- **Prefer caller-saved registers for promoted vars** (RegAlloc): In leaf functions, the color pool puts t5 and unused a-regs before s-regs since they're never clobbered.  Combined with the above, small leaf functions like `f_add1` compile to 5 instructions with zero s-reg save/restore (was 8 with `sd`/`ld`).

### BranchContext: skip store/load for conditions

When a comparison or lazy-boolean expression is used directly as an `if`/`while` branch condition, the IR generator emits a branch on the comparison result instead of storing to memory and loading back.  A `BranchContext` (carrying the true/false labels) is threaded through expression handlers; handlers save labels locally before evaluating children so nested control-flow constructs don't corrupt the context.

### Call-save consolidation

Between consecutive call instructions in the same basic block, the intermediate `RestoreRegister`+`SaveRegister` pair is skipped.  While `registers_saved_` is true, a-register values in hardware are garbage — only the save slots are valid.  Guards at every code path that reads a-regs (`VariableToReg`, `VariableForceToReg`, `DataMove`, `IRStore`, call arg setup) flush or read from save slots.  Writes to a-regs update the save slot directly without a wasted `mv` to the hardware register.  Constant caches (t3/t4) are reloaded via `li` when the restore is deferred.


### Reg alloc color pool

- **Precolored nodes** (function params): a0–a7 (IDs 10–17), set in `reg_alloc.cpp:61-66`.
- **Color pool** depends on whether the function is a leaf:
  - **Leaf functions** (no calls): t5 (ID 30), unused a-regs beyond parameters, then s1–s11, then parameter a-regs as last resort. Since leaf functions never clobber caller-saved registers, t5 and a-regs are safe — no s-reg save/restore in the prologue/epilogue.
  - **Non-leaf functions**: s1–s11 = `{9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27}` (11 registers), plus a0–a7 as fallback.
- The two pools (precolored vs. colorable) are disjoint; `Color()` strips edges between precolored and regular nodes.
- **Move coalescing**: Before coloring, `Coalesce()` attempts to merge move-related virtual registers using Briggs conservative heuristic (merge only if the combined node has degree < K).  Coalesced nodes share the same physical register, eliminating `mv` instructions.  Currently limited by block-level liveness granularity — intra-block phi moves often show spurious interference and aren't coalesced.

### Assembly Generator optimizations (codegen)

- **Immediate folding**: Small constant operands (12-bit signed) in `+`/`-` are folded into `addiw` directly, avoiding `li + addw`.
- **Redundant jump elimination**: Unconditional jumps targeting the immediately-following block (in layout order) are elided.  Branch false-targets that fall through are handled by skipping the trailing `j`.
- **Constant hoisting**: Functions are pre-scanned for large constants (>12-bit).  Up to 2 are loaded into t3/t4 at function entry.  `VariableToReg` returns the cached register instead of emitting inline `li`.  After each call, `RestoreRegister` re-emits `li` to reload.
- **Compare peephole**: `==0`/`!=0` checks use `sltiu`/`sltu` with immediate 1 or x0, avoiding `li t1, 0` entirely.  The peephole fires BEFORE `VariableToReg` so the constant operand is never loaded.
- **Long jumps**: Functions with >40000 instructions or >800 blocks use `lui+addi+jalr` (3 ins, ±2GB range) instead of `j` (1 ins, ±1MB) for intra-function jumps and branches.

## Performance tracking

Cycle counts from the RISC-V simulator across compiler versions are tracked in `performance/`. Each version gets a `N.txt` file. Run `python3 performance/visualize.py` to generate comparison plots.

## RISC-V 64 toolchain (local)

The RV64 linker and cross-compilation libraries are stored under `tools/riscv64-linux/`:
- `lld/bin/` — extracted `lld-21` package (LLVM linker with RISC-V support)
- `sysroot/` — extracted `libc6-riscv64-cross`, `libc6-dev-riscv64-cross`, `libgcc-*-riscv64-cross`, `linux-libc-dev-riscv64-cross`

Source `.deb` files are in `tools/debs/`. To set up on a new machine, extract them with `dpkg-deb -x` into `tools/riscv64-linux/`.

## Testcases submodule

Test cases are in the `testcases/` submodule (`github.com/peterzheng98/RCompiler-Testcases`). After cloning, run `git submodule update --init`.
