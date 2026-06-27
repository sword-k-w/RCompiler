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

CMakeLists.txt currently enables two executables: `code` (links `test/codegen_submit.cpp`, the production pipeline) and `codegen_single` (developer driver). Other test targets (`parser_test`, `semantic_test`, `IR_test`, `codegen_test`, `reg_alloc_test`, `IR_single`, `IR_single_optimized`, `dominator`) are kept in CMakeLists.txt but commented out — uncomment to rebuild.

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
  → FunctionInliner → mem2reg → EliminateCriticalEdge → SCCP → CSE
  → ReplacePhiWithMoves → EliminateEmptyBlocks
  → Preprocessor → MemoryAllocator → ParameterDemoter → RegAlloc
  → ParameterDemoter::FixupAfterRegAlloc → AssemblyGenerator → RISC-V .s
```

The driver lives in `test/codegen_submit.cpp` — that is the authoritative pass order, not the historic diagram.

### Source layout (under `src/`)

- **`lexer/`** — Tokenizer: tries each token function at each position, picks the longest match. Single-char, multi-char, literal, and whitespace/comment tokens are split across separate files.
- **`parser/`** — Recursive-descent parser. Expression parsing uses Pratt parsing for operator precedence. AST nodes each live in `parser/node/`. Template method `Parser::Run<T>()` instantiates parsing for a specific AST node type. In `ExpressionStatementNode`, `ExpressionWithBlockNode` is tried first (before `ExpressionWithoutBlockNode`) to avoid exponential backtracking on deeply nested `if`/`else` chains.
- **`semantic/`** — Scope tree (`scope/`) with separate type/value namespaces. Type system (`type/`) with `ConstValue` for compile-time evaluation. Builtin types/functions defined in `builtin/`.
- **`visitor/`** — AST visitors via `VisitorBase` (~50 Visit methods). Three checker passes: first (scope building, name collection, impl binding), second (type inference, const evaluation), third (type checking). Also `IR_generator/` (AST→IR) and `printer/` (AST pretty-printer).
- **`IR/`** — Three-address-code IR: functions contain blocks, blocks contain instructions. Values are ptr-based (each variable/expression gets a named pointer). Key instruction types: Arithmetic, Branch, Jump, Return, Allocate, Load, Store, GEP, Compare, Call, Phi, Select, Move.
- **`IR_visitor/`** — IR passes via `IRVisitorBase`:
  - `function_inliner/` — inline calls to small functions (≤50 IR instructions, non-recursive).
  - `sccp/` — Sparse Conditional Constant Propagation: lattice (Top/Constant/Bottom) over scalar values, with executable-edge tracking so dead branches don't propagate.
  - `cse/` — within-block CSE for GEP/GEPP: deduplicates address computations with the same `(base, index, type)`, redirecting later uses (including phi incomings) to the first definition.
  - `phi_eliminator/` — replaces phi instructions with explicit moves in predecessors.
  - `empty_block_eliminator/` — removes empty pass-through blocks and phi-merge blocks created by `&&`/`||` chains.
  - `preprocessor/` — pre-computes `allocated_size_`/`align_` on every IR type referenced (incl. nested arrays).
  - `memory_allocator/` — stack frame layout: records sizes (no addresses yet).
  - `parameter_demoter/` — see "Parameter demotion" below.
  - `assembly_generator/` — IR→RISC-V assembly, the final backend.
  - `printer/` — IR pretty-printer.
- **`liveness_analysis/`** — CFG construction, def-use sets, liveness in/out, dominator tree (for SSA construction). `CalcInOut()` uses a worklist algorithm: when a block's IN set changes, only its predecessors are re-queued (since `OUT[pred] = ∪ IN[succ]`). This avoids full fixed-point scans over all blocks, reducing O(n²) to near-linear for deeply nested control flow.
- **`mem2reg/`** — Memory-to-register promotion (alloca→SSA registers with phi insertion) and critical edge elimination.
- **`reg_alloc/`** — Graph-coloring register allocator. Builds interference graph from per-instruction liveness (reverse block walk), promotes scalar variables (i32/i1/ptr) into registers. Uses Chaitin-style simplify/spill/select with worklist, plus Briggs conservative move coalescing. Records per-call-site liveness so call sites can carry a `dead_a_regs_mask_`.
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

### IR optimization passes

- **`FunctionInliner`** — bottom-up inliner. For each non-recursive callee with ≤`kMaxInlineInstructions` (=50) live IR instructions, all call sites are cloned in: blocks are re-IDed, names are remapped through a per-caller `NameAllocator`, struct/array-return callers receive a `pass..pointer` GEP. Runs to fixpoint with per-function `inline_sets` so mutually-recursive cycles don't expand unbounded.
- **`SCCP`** — Sparse Conditional Constant Propagation. Lattice cells are `Top`/`Constant(i32)`/`Bottom`; the executable-edges set keeps unreachable branches from poisoning the lattice. Constant operands fold into instructions, dead branches become unconditional jumps, and Phi inputs from unexecutable edges are dropped.
- **`CSE`** — within-block CSE restricted to GEP/GEPP. For each block, walks instructions building a key from `(base, [index], TypeKey)`; matches redirect future uses (and phi incomings) to the original. `EnsureTypeSize()` pre-computes `allocated_size_`/`align_` before the duplicate is dropped, because later passes assume the Preprocessor visited every GEP type.

### Parameter demotion (`parameter_demoter/`)

A two-step trick that gives parameters the same coloring freedom as ordinary virtuals while keeping the ABI-mandated a-reg arrival.

1. **`Run`** (before RegAlloc): for every parameter currently in `a0–a7`, prepend a `Move %x.mv ← %x` to the entry block, set the move's `storage_type_=kMemory`, and rename every use of `%x` in the function body to `%x.mv` (the demotion moves themselves are skipped during renaming so they keep referencing the a-reg). RegAlloc then promotes `%x.mv` like any other variable — typically into an s-reg, t5, or a free a-reg, sometimes spilling.
2. **`FixupAfterRegAlloc`**: after RegAlloc, walk every demotion move that is still `kMemory` and copy its assigned stack slot from `func->variable_storage_` into the move's `address_` field, since the address was unknown at insert time.

Without this, parameter a-regs interfere with every value live across `SaveRegister`, and the allocator is forced to choose between leaking parameter values into s-regs or constantly spilling.

**Demotion-move coalescing**: when the demoted `%x.mv` doesn't cross any call (every leaf function, plus any non-leaf where the parameter is dead before the first call), the Briggs coalescer merges `%x.mv` back into `%x`'s precolored a-reg. The entry-block `mv` becomes a self-move and is elided. Two pieces are needed for this to work:

- The edge-build loop skips the interference edge between a move's def and its src (otherwise block-level liveness creates a spurious edge: at the move, both src and def appear in the live set, so the standard "def conflicts with everything live" rule wrongly marks them as conflicting).
- `Coalesce` handles precolored moves specially: if `src` is precolored, swap so the precolored side is the survivor (color propagation goes from survivor to coalesced). Skip the coalesce if the virtual side is in `call_crossing_vars_` — forcing the value back into its caller-saved a-reg would require save/restore at every call site, defeating demotion.

### RISC-V temp register conventions (codegen)

- **t0**: arith (operand1), compare aux (sub, sltu, slt), data move, call args
- **t1**: arith (operand2), store const, GEPP (slli, li), saved/restored around calls
- **t2**: arith result (kMemory), GEPP (mul)
- **t3**: **constant cache** — pre-loaded at function entry with a large (>12-bit) constant. Survives calls via `li` reload (`ReloadConstCache`, emitted after every call).
- **t4**: **constant cache** — second pre-loaded constant slot.
- **t5**: long jumps for large functions (`lui+addi+jalr`); also handed to the reg-alloc pool — for **leaf** functions unconditionally, and for **non-leaf** functions to virtuals not live across any call (tracked by `call_crossing_vars_` in the interference graph).
- **t6**: `PrintIA`/`PrintIStar`/`PrintMem` fallback for out-of-range immediates; also scratch for address computation when using cached constants (`add t6, t3, sp`).

Constant cache: per-function frequency analysis picks the 2 most-used large (>12-bit) constants and emits `li t3, ..; li t4, ..` at function entry. The scan covers IR instruction operands (arithmetic, compare, store-const) AND **kMemory stack offsets** (`current_stack_ - addr` for every kMemory variable that appears as an operand) AND a-reg/ra save-slot offsets (weighted by call count). `VariableToReg`/`PrintMem`/`PrintIA`/`PrintIStar` consult `const_cache_`: cached immediates avoid `li t6` emission (e.g., `add t6, t3, sp` replaces `li t6, <imm>; add t6, t6, sp`). After every call, `ReloadConstCache` re-`li`s the cached values; the map is never cleared mid-function (only reset at function entry). `RestoreRegister` calls `ReloadConstCache` first so its a-reg/ra reloads can use the cached offsets for large stack slots.

### Compare→branch fusion (codegen)

A pre-scan in `Visit(IRFunctionNode)` populates `fused_compares_` with every compare whose result feeds an immediately-following branch and has exactly one use in the function. Phi-elimination moves between the compare and the branch are stepped over only when they don't alias any compare operand (checked at both SSA-name and post-RegAlloc physical-register level — coalescing can put distinct SSA names in the same register).

The compare visitor sees the compare is fused and skips emission entirely (no `slt`/`sltu`/`sub`/`xori`), stashing the IR node in `pending_fused_cmp_`. The branch visitor then emits a single `blt/bge/beq/bne/bltu/bgeu` directly using the compare's operands — with operand swap for `kSgt`/`kSle`/`kUgt`/`kUle`, and with the opposite op (`blt↔bge`, etc.) when the true target is the next block in layout order (taken-on-false fall-through). Savings per fused pair:

- `kEq`/`kNe`: 3 instructions → 1 (`sub; sltiu; bnez` → `beq`).
- `kSlt`/`kUlt`/`kSgt`/`kUgt`: 2 → 1 (`slt; bnez` → `blt`).
- `kSle`/`kSge`/`kUle`/`kUge`: 3 → 1 (`slt; xori; bnez` → `bge`).

### Block layout optimization (codegen)

Before emission, `Visit(IRFunctionNode)` reorders `node->blocks_` via a DFS from block 0:

- After placing a block, follow its terminator: `jump` → place the destination next; `branch` → place the **false** target next (so `bnez`/`beqz` falls through on the common path).
- Any unreached blocks are appended at the end.

Combined with the redundant-jump elimination (an unconditional jump targeting the immediately-following block in layout order is elided), this turns most jumps into fall-throughs.

### Phi handling

Phis in the same block are calculated in parallel (see `codegen/phi_topo.cpp`).

### Stack frame layout (codegen)

The frame is laid out so that the most-accessed regions sit closest to sp, where load/store offsets fit in a 12-bit immediate (range `-2048..2047`) and lower to a single `sd/ld off(sp)` instruction. Anything past the 12-bit window requires `li t6, off; add t6, t6, sp; sd/ld 0(t6)` (3 instructions) and is reserved for cold variables and the array-data zone.

```
High addresses (sp + total_stack):
  ┌──────────────────────────────┐
  │                              │
  │ variables and allocas         │ ← assigned by RegAlloc:
  │  (cold scalars,               │   1. allocas (sorted by freq, hot last
  │   array data,                 │      so the hottest sit closest to sp)
  │   hot scalars near the        │   2. kMemory scalars (sorted by freq,
  │   low end of this band)       │      hot last → small sp-offsets)
  │                              │
  ├──────────────────────────────┤ ← sp + s_save + a_save
  │ ra save              (off +N)│ ┐ a-reg/ra save area
  │ a(N-1) save                  │ │ Save slot for a-reg i:
  │ ...                          │ │   sp + s_save + 8*i
  │ a0 save     (off s_save + 0) │ │ Save slot for ra:
  │                              │ ┘   sp + s_save + 8*current_a_reg_used_
  ├──────────────────────────────┤ ← sp + s_save (non-leaf only)
  │ s1 save             (off  0) │ ┐ prologue / epilogue: s-regs packed
  │ s2 save             (off  8) │ │ tightly at sp + 0, 8, 16, ...
  │ ...                          │ ┘
  └──────────────────────────────┘
Low addresses (sp):
```

- `s_save = 8 * |used_s_regs|`.
- `a_save = 8 * (a_reg_used_cnt_ + 1)` for non-leaf functions (`has_calls_`), else 0.
- `total_stack = stack_size_ + s_save + a_save`, rounded up to the nearest multiple of 16 for RISC-V ABI compliance.
- `current_stack_ = total_stack`. Variable access: `sp + (current_stack_ - address)`.
- Save slots: `sp + save_area_base_ + 8*i`, where `save_area_base_ = s_save`. With `a_save ≤ 72`, every save/restore lands in the cheap 12-bit window — the same offset that used to cost 3 instructions now costs 1.
- t-regs are pure scratch (caller-saved, dead after each instruction); no save space reserved.
- t3/t4 constants are reloaded via `li` after calls, not saved to memory.

### Memory allocation

MemoryAllocator records sizes without assigning addresses. After RegAlloc promotes scalar variables to registers, only variables still in memory (`kMemory`) receive contiguous stack addresses. Promoted variables never occupy stack space, so no compaction is needed. `i32` slot size is **4 bytes** (not 8), to keep struct/array storage and stack memcpy budgets tight.

### Frequency-aware address assignment (RegAlloc step 6)

Stack addresses are assigned in two phases, both sorted by access frequency (ascending — hot last → largest address → smallest sp-offset):

1. **Allocas** first (low addresses → high sp-offsets, expensive). Hottest allocas placed last so their data sits closest to sp.
2. **kMemory scalar variables** last (highest addresses → smallest sp-offsets, cheap).

Frequency is counted by walking every non-removed instruction's `def_`/`use_` sets, weighted by a per-block multiplier: 10× for blocks that look like loop headers (any predecessor with a higher block id, i.e. a back-edge), 1× otherwise. The check is cheap and approximates "this block is inside a loop" using IR-generation order, which is roughly topological for `while`/`loop` constructs.

The kMemory-scalar zone is typically a few hundred bytes for run_filter-sized functions, so the entire scalar zone — including every array's pointer slot — packs into the cheap [0, 2047] sp-offset range. Each pointer-slot access (one per array-element read) drops from 3 instructions to 1.

### Leaf function optimizations

Two cooperating optimizations eliminate unnecessary stack frame overhead for leaf functions (functions with no calls):

- **Skip a-reg/ra save area** (IR / MemoryAllocator): Leaf functions whose parameters all fit in a0–a7 don't need the 72-byte caller-save area. Previously every function allocated this area, causing simple leaf functions to waste ~144 bytes of stack with s-reg save/restore. After this change, leaf function stack frames shrink to ~16 bytes (just the s-reg save area, if any s-regs are used). Struct/array Load/Store/Move instructions that lower to `builtin_memcpy` are also detected as calls.
- **Prefer caller-saved registers for promoted vars** (RegAlloc): In leaf functions, the color pool puts t5 and unused a-regs before s-regs since they're never clobbered. Combined with the above, small leaf functions like `f_add1` compile to 5 instructions with zero s-reg save/restore.

### BranchContext: skip store/load for conditions

When a comparison or lazy-boolean expression is used directly as an `if`/`while` branch condition, the IR generator emits a branch on the comparison result instead of storing to memory and loading back. A `BranchContext` (carrying the true/false labels) is threaded through expression handlers; handlers save labels locally before evaluating children so nested control-flow constructs don't corrupt the context.

### Call-save consolidation

`SaveRegister`/`RestoreRegister` is **deferred to basic-block terminators** rather than bracketing each call. A call only clobbers caller-saved registers, so after `SaveRegister` runs once, `registers_saved_` is left true across the rest of the block: every subsequent call (real calls **and** the `builtin_memcpy`s emitted inline by Load/Store/Move/struct-arg setup — including chains of them separated by GEPs and other a-reg-safe ops) reuses the same save and skips its own restore+save pair. The restore is flushed exactly once, at the block exit (`Visit(IRBranchInstructionNode)`/`Visit(IRJumpInstructionNode)`/`Visit(IRReturnInstructionNode)` all call `FlushSavedRegisters()` first), so hardware a-regs/ra are valid for successors and `ret`.

While `registers_saved_` is true, hardware a-regs hold call garbage — the **save slots are the source of truth**. Three things keep that consistent across code that previously assumed live hardware a-regs between calls:

  - **Reads** go through slots, not hardware: `VariableToReg`/`VariableForceToReg`/`DataMove`/`IRStore`/call-arg setup load from the save slot when the operand is an a-reg, or flush if they need the hardware value.
  - **Results** route through a slot too: `GetResultReg` returns a t-reg (not `x<addr>`) for an a-reg result when `registers_saved_` holds, so the producing instruction computes into the t-reg and `RegToVariable` writes it to the slot — otherwise the result would land in the hardware a-reg, invisible to the slot-based reads and clobbered by the next restore.
  - **Const cache stays valid**: `ReloadConstCache` (the extracted t3/t4 reload loop) runs after *every* call site, and the const-cache map is never cleared mid-function (only reset at function entry). Clearing it would make the post-call reload a no-op while later `SaveRegister`/`RestoreRegister` `EmitMem`s still rely on the cached offsets for stack slots.

### Per-call dead a-reg masks

RegAlloc snapshots the live set at every call site (in `call_live_sets`) AFTER killing defs but BEFORE genning uses. From that it computes a per-call `dead_a_regs_mask_` (bit `i` set ⇔ a-reg `i` holds no live value at this call). `Visit(IRCallInstructionNode)` consults `dead_a_regs_mask_` before `SaveRegister`: any a-reg listed as dead has its `a_reg_valid_` left invalidated and skips the `sd` to its save slot for this call (and the matching `ld` in the next flush). This is what makes a-regs viable for non-parameter virtuals — without it, every call would unconditionally save all 8 a-regs.

### Reg alloc color pool

- **Precolored nodes** (function params): a0–a7 (IDs 10–17).
- **Color pool**:
  - **Leaf functions** (`!has_calls_`): t5 (ID 30), unused a-regs beyond parameters, then s1–s11 (IDs 9, 18–27), then parameter a-regs as last resort. Caller-saved registers are safe because no call ever clobbers them.
  - **Non-leaf functions**: `{9, 18..27}` (s1–s11) then `{10..17}` (a0–a7) then `{30}` (t5). For each variable in `call_crossing_vars_` (live at ≥1 call site), the interference graph strips edges to t5 — so t5 is only available to virtuals whose live range doesn't cross any call. With per-call dead masks (above), a-regs can be coalesced into non-parameter virtuals too.
- The two pools (precolored vs. colorable) are disjoint; `Color()` strips edges between precolored and regular nodes.
- **Move coalescing**: Before coloring, `Coalesce()` merges move-related virtual registers using Briggs conservative heuristic (merge only if the combined node has degree < K). Two helpers make this fire on more cases:
  - **Spurious-edge skip**: when the edge-build loop processes `dst = mv src`, it skips the def↔src interference edge. Block-level liveness would otherwise create one (both src and dst appear in the live set at the move instruction), wrongly blocking the coalescer.
  - **Precolored-aware coalesce**: when one side of the move is a precolored a-reg (parameter-demotion moves), `Coalesce()` swaps so the precolored side is the survivor, propagating the precolor into the virtual. Refuses if the virtual is call-crossing — letting it sit in the caller-saved a-reg would cost a save/restore at every call site.

### Assembly Generator optimizations (codegen)

- **Immediate folding**: Small constant operands (12-bit signed) in `+`/`-` are folded into `addiw` directly, avoiding `li + addw`.
- **Redundant jump elimination**: Unconditional jumps targeting the immediately-following block (in layout order) are elided. Branch false-targets that fall through are handled by skipping the trailing `j`.
- **Constant hoisting**: see "constant cache" above.
- **Compare peephole**: `==0`/`!=0` checks use `sltiu`/`sltu` with immediate 1 or x0, avoiding `li t1, 0` entirely. The peephole fires BEFORE `VariableToReg` so the constant operand is never loaded.
- **Compare→branch fusion**: see dedicated section above.
- **Block layout**: see dedicated section above.
- **Long jumps**: estimated assembly size = `total_ins*3 + call_cnt*3 + blocks*3`. Functions with `est_asm > 8000` or `blocks > 500` use `lui+addi+jalr` (3 ins, ±2GB range) instead of `j`/`beqz` for intra-function jumps and branches.

## Performance tracking

Cycle counts from the RISC-V simulator across compiler versions are tracked in `performance/`. Each version gets a `N.txt` file. Run `python3 performance/visualize.py` to generate comparison plots.

## RISC-V 64 toolchain (local)

The RV64 linker and cross-compilation libraries are stored under `tools/riscv64-linux/`:
- `lld/bin/` — extracted `lld-21` package (LLVM linker with RISC-V support)
- `sysroot/` — extracted `libc6-riscv64-cross`, `libc6-dev-riscv64-cross`, `libgcc-*-riscv64-cross`, `linux-libc-dev-riscv64-cross`

Source `.deb` files are in `tools/debs/`. To set up on a new machine, extract them with `dpkg-deb -x` into `tools/riscv64-linux/`.

## Testcases submodule

Test cases are in the `testcases/` submodule (`github.com/peterzheng98/RCompiler-Testcases`). After cloning, run `git submodule update --init`.
