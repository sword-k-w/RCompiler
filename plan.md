# Register Allocation via Graph Coloring — Implementation Plan

## Overview

Replace the current trivial stack-spilling allocator (everything goes to memory) with a graph-coloring register allocator that promotes variables into physical RISC-V registers.

## Status

### Step 0: Decouple Storage from Instructions ✓ (done, b6dc421)

Added `variable_storage_` map to `IRFunctionNode`. Storage is now variable-centric: MemoryAllocator populates the map with conditional allocation (first def allocates, later defs reuse), and `GetVariableAddress` reads from the map directly instead of dynamic_cast-ing through the defining instruction. Removed shadow `storage_type_`/`address_` fields from `IRCallInstructionNode`. All 50 codegen tests pass.

### Step 1: Replace Phi with Move Instructions ✓ (done, 453b205)

Added `IRMoveInstructionNode` and a `ReplacePhiWithMoves` pass that eliminates phi nodes by inserting explicit move instructions in predecessor blocks (before the terminator), using PhiTopo for parallel copy ordering. Updated all visitors (Preprocessor, MemoryAllocator, CFGBuilder, IRPrinter, AssemblyGenerator) to handle moves instead of phis. Simplified branch/jump visitors in AssemblyGenerator — phi/PhiTopo logic removed. Pipeline: `ReplacePhiWithMoves → Preprocessor → MemoryAllocator → ...`

Fixed cycle-break temp bug: the temp variable for phi copy cycles was allocated twice with different names (save to temp1, load from temp2). Now allocates once and reuses.

Also fixed terminator lookup: `ReplacePhiWithMoves` no longer assumes `instructions_.back()` is the terminator; it scans for the last branch/jump in case later passes (e.g. mem2reg) appended instructions after the terminator.

### Step 2: Register Allocation via Graph Coloring ✓ (done)

**Implemented:**
- `InterferenceGraph` class: adjacency list, precolored nodes, Chaitin-style simplify/spill/select coloring (`Color()`)
- Precolored nodes (a-regs) are removed from the worklist upfront and their edges are stripped before coloring, since the a-reg and s-reg color pools are disjoint. `Color()` asserts this.
- `RegAlloc` visitor: re-runs CFGBuilder per function, computes promotable variable set upfront, builds interference graph from per-instruction liveness (reverse block walk following `CFG::CalcInOut` pattern), colors with s1–s11 pool (11 registers, s0 skipped due to t1 save-slot conflict), rewrites `storage_type_`/`address_` on colored instructions
- `IsPromotableType`: filters complex types (structs, arrays) — only promotes arith/neg/comp/select/gep/gepp/load/call/move with simple base types (i32, i1, ptr) and non-array lengths
- s-reg save/restore in AssemblyGenerator prologue/epilogue (using `used_s_regs_` on `IRFunctionNode`)
- `RegToVariable` fix: added missing `x` prefix on `mv` destination register

**Bugs fixed:**
- InterferenceGraph `Color()`: infinite loop when only precolored nodes remain in active set — now erases precolored nodes upfront
- Liveness computation: initial `live` set from `block->out_` now correctly filters by `promotable_vars` to avoid non-promotable `not_alloc` variables (array types) persisting in the live set and creating spurious interference edges
- Stack frame layout: s-reg saves now go at the **bottom** of the frame (sp + 0, sp + 4, ...), a-reg/ra/t1 saves stay at the **top** (within the MemoryAllocator's 64-byte reserved area). The frame is extended by `4 * |used_s_regs|` to make room at the bottom. This avoids any overlap between the two save areas.
- `ReplacePhiWithMoves` terminator finding: scans for branch/jump rather than assuming `back()`

### Step 3: Assembly Generator s-reg save/restore ✓ (done as part of Step 2)

### Step 4: Wiring ✓ (done as part of Step 2)

RegAlloc pass inserted after MemoryAllocator, before AssemblyGenerator in both `codegen_test.cpp` and `codegen_single.cpp`.

## Remaining Work

1. ~~**Move coalescing**: After coloring, eliminate no-op moves where source and dest share the same register~~ ✓ Done (Briggs conservative coalescing in `reg_alloc/`).  Currently limited by block-level liveness — intra-block phi moves often show spurious interference.
2. **Instruction-level liveness**: Block-level CFG liveness causes spurious interference between old/new versions of the same variable within a block (e.g. `%phi..1` used by add, then redefined by move).  Finer-grained liveness would allow coalescing of these move pairs, eliminating `mv` in tight loops.
3. **Expand color pool**: Consider adding a0–a7 to the pool with proper save/restore around calls
4. **Stack frame reservation**: The MemoryAllocator reserves 120 bytes at the top for a-reg/ra/t1 saves. Currently sufficient for typical cases; dynamic reservation may be needed for high register pressure.
5. **Codegen peephole**: `addw tX, sA, sB; mv sA, tX` → `addw sA, sA, sB` when the temp isn't reused. Could be done in `Visit(IRMoveInstructionNode)` by rewriting the preceding arithmetic's destination register.
