# Compiler Optimizations

Summary of optimizations implemented with Claude, from `f99bb6c` (RVO for struct/array returns — the first IR-level optimization) through `89a0349` (HEAD, deferred kMemory stores). 75 commits total.

## IR Generation Optimizations

### RVO and Direct-Write for Struct/Array Literals
Three cooperating fields eliminate temporary allocations and `memcpy` for struct/array literals:
- **`let_target_`**: When a `let` statement's RHS is a struct/array literal, the literal writes directly into the pattern's IR name, skipping the temp+memcpy. *(commit e20352a)*
- **`rvo_target_`** + **`in_return_`**: When a function has a struct/array return type, the caller passes a return buffer pointer (`%pass..pointer`). In `Return()`, if the expression's IR name already equals `rvo_target_`, the memcpy is skipped. *(commits e6224c8, f99bb6c)*
- **Nested propagation**: When a struct/array field is itself a struct/array literal, the parent pre-computes a GEP into the target and sets it as `let_target_` for the child, propagating the optimization recursively. *(commit 22ebfe5)*

### Array Repeat Initialization
- **Loop blocks**: Replaced unrolled `[val; N]` array repeat initialization with a compact 4-block IR loop (header/body/end). Eliminates O(N) Allocate+Load+Store per element — the scalar is loaded once and stored in a loop regardless of array size. *(commit 582ce8f)*
- **Running pointer strength reduction**: Inside `[val; N]` loops, replaced `GEPP(base, idx, elem_size)` with a running pointer chain (`ptr += elem_size`). Eliminates `li+mul+add` per iteration, reducing to a single `addi`. Extended to `[struct_val; N]` copies and explicit `[a, b, c, ...]` lists. Total: −31,378 asm lines (−12.6%) across 50 tests. *(commits 97ad195, d7342d4)*

### Branch Condition Optimization
When a comparison or lazy-boolean expression is used directly as an `if`/`while` branch condition, the IR generator emits a branch on the comparison result instead of storing to memory and loading back. A `BranchContext` carries true/false labels through expression handlers; handlers save labels locally before evaluating children so nested control-flow constructs don't corrupt the context. *(commit 0fc3e95)*

## IR Optimization Passes

### SCCP (Sparse Conditional Constant Propagation)
Wegman-Zadeck style constant propagation operating on SSA-form IR with phi nodes intact. Two-worklist fixpoint (flow + SSA) tracks constant lattice values (TOP/CONSTANT/BOTTOM) for every variable and executability for every block. Folds all-constant arithmetic/compare/select, converts constant-condition branches to unconditional jumps, removes unreachable code. Runs after EliminateCriticalEdge, before ReplacePhiWithMoves. *(commit 8d0b9e1)*

### Function Inlining
Replaces call instructions with the callee's body when heuristically profitable (< 50 instructions). Clones callee blocks, renames locals with per-function prefixes, redirects allocas into the caller's entry block, splits blocks at call sites, and rewrites returns as jumps to continuation blocks. Runs after IR generation, before mem2reg. *(commit 0e52779)*

### GEP Chain Folding
- **Zero-offset**: Detects GEP(const) with index 0 (first struct field / array element 0), replaces all uses with the ptrval and removes the GEP entirely. Eliminates useless `mv` instructions from nested accesses like `a[i].x.y[j]`. Example: 7 insns → 5 insns (−28%). *(commit 1d6a149)*
- **Non-zero constant**: Walks backward from true terminals through single-use intermediate GEPs, collapsing chains like `addi rd, base, 8; addi rd, rd, 8` into `addi rd, base, 16`. Instruction count on comprehensive1 dropped from 6200 to 4898 (−21%). *(commit d42dc0f)*

### Return Consolidation
Each function now has one return block containing the epilogue (s-reg restore, stack adjust, ret). All return sites jump there instead of duplicating the epilogue. Scalar return values route through a canonical `%return.val` alloca promoted by mem2reg. *(commit 2bb17fe)*

## Control Flow Optimizations

### Empty Block Elimination (4 passes)
After phi elimination and inlining create empty/pass-through blocks:
- **Pass 1**: Eliminate truly empty blocks (no instructions) by redirecting predecessors through jump chains. *(commit 4a08b33)*
- **Pass 2**: Single-move jump threading — blocks containing only Move+Jump are eliminated by duplicating the Move into each predecessor and redirecting the Jump. Cleans up pass-through blocks from ReplacePhiWithMoves and the inliner. *(commit 55436eb)*
- **Pass 3**: Phi-merge block elimination — blocks with only a branch on a phi variable (from `&&`/`||` chains) are threaded: the predecessor's Jump becomes a Branch testing the Move's source directly, the Move becomes dead, and the merge block is removed. *(commit bb807de)*
- **Pass 4**: Dead-block chain resolution — after SCCP/inlining removes all instructions from a block, follows jump chains through empty blocks to find the first live target and redirects all predecessors. *(commit 68a8b30)*

### Block Layout Reordering
DFS-based block layout optimization: for jumps, the target is placed immediately next; for branches, the false target is preferred (so `bnez`/`beqz` falls through on the common path). Eliminates unnecessary `j` instructions. *(commit 8c1ab61)*

### Empty Block Awareness in Assembly Generator
`next_block_map_` skips empty blocks so predecessors see the correct fall-through target, enabling jump elision even when eliminated blocks remain in the block vector. *(commit 5431d1d)*

## Register Allocation

### Color Pool Expansion
Added a0–a7 to the color pool (from 11 to 19 registers: s1–s11 + a0–a7). Precolored parameters properly interfere with promotable variables. *(commit 0cf2f27)*

### Briggs Conservative Move Coalescing
Move-related virtual register pairs are merged before coloring if they don't interfere and the merged node has degree < K (guaranteeing colorability). Coalesced nodes share the same physical register, eliminating `mv` instructions. *(commit 8dc8709)*

### Leaf Function Register Preference
In leaf functions, the color pool puts t5 and unused a-regs before s-regs since they're never clobbered. Combined with stack frame savings, small leaf functions compile with zero s-reg save/restore. *(commit 289cc6b)*

### Spill Candidate Ordering
Pre-sort spill candidates by static weight (uses+defs ascending, then degree descending) instead of random selection. Avoids O(V²) per-iteration scan while producing better spill choices than random. *(commit 0bae12d)*

### RegAlloc Performance (Data Structures)
- BitSet for liveness in/out sets — word-level OR/AND/NOT processes 64 variables at a time *(commit c039bfc)*
- `std::unordered_map` replaces `std::map` for adjacency and string-keyed lookups *(commits b00f8ed, e1f326d)*
- Worklist-based color simplify instead of O(V) linear scan each iteration *(commit 6d46354)*

## Stack Frame Optimizations

### Leaf Function Frame Elimination
Leaf functions (no calls, all parameters fit in a0–a7) skip the caller-save area entirely. Simple leaf function stack frames shrink from ~144 to ~16 bytes (just the s-reg save area). Struct/array operations that lower to `builtin_memcpy` are also detected as calls. *(commit a7f9839)*

### Deferred Stack Address Assignment
MemoryAllocator records sizes without assigning addresses. After reg_alloc promotes scalars to registers, only variables still in memory (kMemory) receive contiguous stack addresses. Promoted variables never occupy stack space — no compaction needed. *(commit c02d591)*

### Tightened Save Area
Instead of always reserving 72 bytes for the a-reg/ra area, only reserve `8 + 8 × a_reg_used_cnt_` for functions whose parameters all fit in registers. gcd test: stack 96→48, main: 96→32. *(commit 0acee5e)*

### Packed Save Area
Removed all t-reg slots from the save area (t-regs are caller-saved scratch, dead after each instruction). a-regs and ra are now packed at offsets 8–72 instead of scattered across 8–120. Frame size reduced by 48 bytes per function with calls. *(commit b65e66f)*

## Assembly Code Generation

### Peephole Optimizations
- **Compare**: eq/ne use `sub+sltiu/sltu` (2 insns) instead of `slt+slt+or+xori` (3–4 insns). `==0` uses `sltiu` (1 insn), `!=0` uses `sltu x0`. *(commit 748912b)*
- **Compare peephole ordering**: `==0`/`!=0` checks fire BEFORE `VariableToReg`, avoiding wasted `li t1, 0` when the peephole uses `sltiu` with immediate 1. Merged compare aux register from t3 into t0. *(commit d1dac24)*
- **GEP with power-of-2**: Uses `slli` instead of `li+mul`. *(commit 748912b)*
- **Constant negation**: Folds `li+N; neg` into single `li -N`. *(commit 748912b)*
- **Immediate folding**: Small constant operands in `+`/`-` folded into `addi/addiw`, saving one `li` per folded op. *(commit b0e690d)*
- **Commutative operand1**: After SCCP makes operand1 a constant, commutative `+` folds `c + x` into `addi/addiw` instead of emitting a separate `li`. *(commit 4c7674e)*
- **Power-of-2 strength reduction**: `li+mulw` → `slliw`, `li+divuw` → `srliw`, `li+remuw` → `andi`, `divw` → `sraiw`+correction for signed division. *(commit f422ad2)*

### Redundant Jump Elimination
Unconditional jumps targeting the immediately-following block (detected via `next_block_map_`) are elided. For branches whose true/false target is the next block, the trailing `j` is skipped or the branch is inverted to `beqz` for fall-through. *(commit b0e690d)*

### Long Jumps
Functions exceeding thresholds use `lui+addi+jalr` (3 insns, ±2GB) instead of `j` (1 insn, ±1MB). Branches split into a local `bnez`/`jump` pair so the B-type instruction always targets within ±4KB. Threshold tightened to account for const-cache overhead: >8000 estimated asm insns or >100 blocks (was >40000 IR-only insns / >800 blocks). *(commits 043b986, f87684b, 529d598)*

### Branch Codegen Simplification
Use `bnez+j` instead of `beqz+j+j` — the assembler/linker can relax `bnez` to `beqz+j` when out of range, saving one jump per conditional branch. *(commit 260bcc0)*

### Constant Cache (t3/t4)
Pre-scan each function for large constants (>12-bit) using frequency analysis. The 2 most frequent are loaded into t3/t4 at function entry and survive calls via `li` reload. `VariableToReg`, `PrintMem`, `PrintIA`, and `PrintIStar` all check the cache, avoiding inline `li` emission. Scan includes IR operands **and** stack offsets from SaveRegister/RestoreRegister. Cached immediates avoid `li t6` (e.g., `add t6, t3, sp` instead of `li t6, <imm>; add t6, t6, sp`). Reduces total assembly by ~3.5% (413k→399k lines across 50 tests). *(commits 008b3e8, 7ff6ead, 809008d)*

### Call-Save Consolidation

**Phase 1 — Basic consolidation**: SaveRegister/RestoreRegister consolidated across consecutive calls in the same basic block. Between back-to-back calls, the intermediate restore+save pair is skipped — a-reg values stay valid in save slots. *(commit 5643cc7)*

**Phase 2 — Deferred to block terminators**: Extended to all calls (not just consecutive ones). SaveRegister stays idempotent; restore is deferred to basic-block exits (branch/jump/return), which call `FlushSavedRegisters()` first. Between calls, `registers_saved_` stays true so all reads/writes route through save slots. Asm size on memcpy-heavy cases drops 28–47% (case 14: 66614→38564, case 19: 50262→26301). *(commit fadf075)*

**Phase 3 — Per-register lazy save/restore**: Replaced the single `registers_saved_` boolean with per-register `a_reg_valid_[8]` tracking. Each a-reg is independently valid or invalid. On first read of an invalid a-reg, restores from save slot; subsequent reads use hardware directly. Registers never touched between calls incur zero overhead. *(commit e90932a)*

**Phase 4 — Redundant flush avoidance**: `FlushSavedRegisters` skips `ReloadConstCache` in call-free blocks where t3/t4 were never clobbered. Eliminates redundant `li t3/t4` at every exit point in functions with many basic blocks. *(commit a9cc159)*

### Call Result Direct Use
After a call, a0 hardware still holds the return value. Use a0 directly as the source in `RegToVariable` instead of stashing to t0 first. Saves one `mv` per non-void call (−1,133 `mv`, −0.4% total instructions). *(commit 37d064e)*

### Deferred kMemory Stores
`RegToVariable(kMemory, addr, t_reg)` defers the `sd` instead of emitting eagerly. `VariableToReg` checks the deferred slot and reuses the register directly (skipping the `ld`). `BeforeWrite(rd)` flushes the deferred store before any instruction overwrites the register. Eliminates the store-then-immediate-reload pattern where an alloca computes an address into t1, `RegToVariable` emits `sd`, and the very next instruction's `VariableToReg` emits `ld`. *(commit 89a0349)*

### Label & Comment Stripping
- Debug comments stripped, unreferenced labels elided *(commit 897da86)*
- Labels mapped to short global integer IDs (`.L<N>`) instead of verbose `.L<funcname>_<blockid>` *(commit 0927d8f)*

## Liveness Analysis & SSA Construction Performance

### Worklist Algorithm for CalcInOut
Replaced full fixed-point iteration (rescanning all blocks every round) with a worklist algorithm: when a block's IN set changes, only its predecessors are re-queued. 47x speedup on deeply nested if/else chains (18s→0.38s at depth 80). *(commit 2f2302d)*

### Postorder Traversal
Initial liveness pass uses DFS postorder so successors are processed before predecessors (matching backward dataflow direction), reducing repetitive propagation. For deeply nested while loops (depth=100): ~3.5s→~0.6s. *(commit 50a9b14)*

### BitSet for Liveness Sets
`std::set<uint32_t>` replaced with word-level BitSet (`std::vector<uint64_t>`). Bitwise OR/AND/NOT process 64 variables at a time. Uses ctz-based bit iteration for sparse scanning. *(commit c039bfc)*

### Batched SSA Rename
Single-pass dominator tree traversal walks all promoted variables simultaneously instead of one DFS per variable. O(blocks) instead of O(vars×blocks). *(commit e8f2ece)*

### Per-Block Instruction Index
Pre-builds a map from pointer name to referencing instructions, enabling O(1) lookup in PhiDFS instead of scanning all instructions with `dynamic_cast` and string comparison. *(commit 95986d4)*

### Pre-Computed Def Blocks
`AddPhi` uses a `def_blocks_` map built during CFG construction instead of scanning every block per variable. PhiDFS skips instruction scan for blocks whose def/use sets don't contain the variable. *(commit d2ad702)*

### Two-Phase Phi Replacement
Phase 1: run DFS for each promotable alloca, accumulating load→value mappings without clearing between variables. Phase 2: a single `PhiRewriteAll` pass rewrites all instruction operands. Eliminates O(vars×blocks) rewrite pass. *(commit 0c882fd)*

### Redundant CFG Work Skipped
- `EliminateCriticalEdge` skips both `CalcInOut()` and def/use set building (it only needs block adjacency). *(commits c9cc45c, ef0f48c)*
- CFGBuilder def/use accumulation uses `vector+BitSet` instead of `std::set`, reducing allocations. *(commit da221c3)*

### Data Structure Improvements
- Pre-compute promotable alloca candidates in a single scan over all instructions — O(blocks) instead of O(vars×blocks) *(commit c039bfc)*

## Bug Fixes

- **GEP offset alignment**: Fixed 1-byte struct member size in GEP offset calculation *(commit c7cf205)*
- **Ptr arithmetic width**: `addw` sign-extends bit 31, corrupting 64-bit pointers. Fixed to emit 64-bit `add/addi/sub` for "ptr" type. *(commit 97ad195)*
- **Save slot consistency**: Multiple fixes for a-reg read/write guards during call-save deferral — writes must mirror to save slots, reads must flush before direct a-reg access *(commits 82b7e6a, 179b5ee, f0e8473, b071984, 2fa9226, 13c2550)*
- **Block layout with empty blocks**: DFS fall-through fix when empty blocks remain after SCCP/inlining *(commit 68a8b30)*
- **Empty blocks in next_block_map_**: Skip empty blocks so predecessors see the correct fall-through target *(commit 5431d1d)*
- **Long-jump threshold**: Tightened to account for const-cache bloat (`li` reloads per call + per block) *(commit 529d598)*
- **stoll crash**: Guard against empty/malformed constant names in IR *(commit ec56ac7)*
