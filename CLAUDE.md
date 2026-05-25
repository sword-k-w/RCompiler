# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A Rust-subset (`.rx`) to RISC-V 32-bit compiler written in C++20. The compiler targets a custom RISC-V simulator called `reimu`.

## Build

```bash
# Configure + build all targets into build/
make build

# Or manually:
mkdir -p build && cd build && cmake .. && cmake --build .
```

The CMake build produces 8 targets: `parser_test`, `semantic_test`, `IR_test`, `codegen_test`, `IR_single`, `IR_single_optimized`, `codegen_single`, `dominator`.

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

Test cases live in `testcases/` (git submodule). Scripts under `script/IR/` and `script/codegen/` drive end-to-end testing through the `reimu` simulator.

## Architecture

### Compiler pipeline

```
Source (.rx) → Lexer → Parser → Semantic (3 passes) → IR Generator → [IR Preprocessor → Memory Allocator → mem2reg → Reg Alloc] → Assembly Generator → RISC-V .s
```

### Source layout (under `src/`)

- **`lexer/`** — Tokenizer: tries each token function at each position, picks the longest match. Single-char, multi-char, literal, and whitespace/comment tokens are split across separate files.
- **`parser/`** — Recursive-descent parser. Expression parsing uses Pratt parsing for operator precedence. AST nodes each live in `parser/node/`. Template method `Parser::Run<T>()` instantiates parsing for a specific AST node type.
- **`semantic/`** — Scope tree (`scope/`) with separate type/value namespaces. Type system (`type/`) with `ConstValue` for compile-time evaluation. Builtin types/functions defined in `builtin/`.
- **`visitor/`** — AST visitors via `VisitorBase` (~50 Visit methods). Three checker passes: first (scope building, name collection, impl binding), second (type inference, const evaluation), third (type checking). Also `IR_generator/` (AST→IR) and `printer/` (AST pretty-printer).
- **`IR/`** — Three-address-code IR: functions contain blocks, blocks contain instructions. Values are ptr-based (each variable/expression gets a named pointer). Key instruction types: Arithmetic, Branch, Jump, Return, Allocate, Load, Store, GEP, Compare, Call, Phi, Select.
- **`IR_visitor/`** — IR passes via `IRVisitorBase`: `preprocessor/`, `memory_allocator/` (stack frame layout), `assembly_generator/` (IR→RISC-V assembly, the final backend).
- **`liveness_analysis/`** — CFG construction, def-use sets, liveness in/out, dominator tree (for SSA construction).
- **`mem2reg/`** — Memory-to-register promotion (alloca→SSA registers with phi insertion) and critical edge elimination.
- **`reg_alloc/`** — Register allocation, currently stub/WIP (interference graph defined).
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

### RISC-V temp register conventions (codegen)

- t0-t1: arith, neg, br, j, load, store, gete, getep, comp, call, sel, alloca
- t2: arith, getep, comp
- t3-t4: comp
- t5-t6: free

### Phi handling

Phis in the same block are calculated in parallel (see `codegen/phi_topo.cpp`).

### Stack frame layout (codegen)

s-reg saves live at the bottom of the frame (near sp), a-reg/ra/t1 saves at the top (within the MemoryAllocator's 64-byte reserved area). The two areas never overlap.

```
High addresses (sp + total_stack):
  ┌──────────────────────────────┐
  │ t1 save             (off  4) │ ┐
  │ ra save             (off 60) │ │ SaveRegister / RestoreRegister
  │ a0 save             (off 28) │ │ fixed offsets from current_stack_
  │ a1 save             (off 32) │ │
  │ ...                          │ ┘
  ├──────────────────────────────┤ ← top of original stack_size_
  │                              │
  │ local variables & spills     │ ← MemoryAllocator allocations
  │                              │
  ├──────────────────────────────┤ ← bottom of variable area (sp + s_save)
  │ s1 save             (off  0) │ ┐ prologue / epilogue: s-regs
  │ s2 save             (off  4) │ │ packed at sp + 0, 4, 8, ...
  │ ...                          │ ┘
  └──────────────────────────────┘
Low addresses (sp):
```

- `s_save = 4 * |used_s_regs|`, `total_stack = stack_size_ + s_save`
- s-regs at `sp + 0, sp + 4, ...` (bottom of frame)
- a-regs at `current_stack_ - 28 - 4*i`, ra at `current_stack_ - 60`, t1 at `current_stack_ - 4` (top of frame)
- Variable access: `sp + current_stack_ - address`; extending sp and `current_stack_` by `s_save` shifts variable area up by `s_save`, leaving the bottom free for s-regs.

### Reg alloc color pool

- **Precolored nodes** (function params): a0–a7 (IDs 10–17), set in `reg_alloc.cpp:61-66`.
- **Color pool** (for promoted vars): s1–s11 = `{9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27}` (11 registers).
- The two pools are disjoint; `Color()` asserts this and strips edges between precolored and regular nodes.

## Performance tracking

Cycle counts from the `reimu` simulator across compiler versions are tracked in `performance/`. Each version gets a `N.txt` file. Run `python3 performance/visualize.py` to generate comparison plots.

## Testcases submodule

Test cases are in the `testcases/` submodule (`github.com/peterzheng98/RCompiler-Testcases`). After cloning, run `git submodule update --init`.
