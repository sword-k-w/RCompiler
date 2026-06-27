# Plan: GCC-style Register Allocation Strategy

## Goal

Adopt GCC's register allocation strategy: prefer caller-saved registers (a0-a7)
over callee-saved registers (s1-s11), with per-callsite liveness analysis so
only live a-regs are saved/restored around each call.

## Expected Impact

| Metric | Current | Target | Rationale |
|--------|---------|--------|-----------|
| Prologue sd | ~70 | ~20 | Fewer s-regs promoted → fewer s-reg saves |
| Call saves sd | ~105 | ~40 | Only save live a-regs at each call |
| Total sd | 236 | ~120 | ~50% reduction in sd/ld |
| Total ld | 292 | ~160 | Matching reduction |

## Current Architecture

```
Color pool (non-leaf): s1-s11 FIRST, then a0-a7
SaveRegister:         saves ALL a0-a7 unconditionally
FlushSavedRegisters:  restores ALL invalid a-regs
```

The problem: s-regs require prologue/epilogue save/restore. a-regs require
save/restore at EVERY call site. Since we save all a-regs unconditionally,
preferring a-regs would increase call overhead. We need per-callsite liveness
to make a-regs cheaper.

## GCC's Architecture (target)

```
Color pool:           a0-a7 FIRST (caller-saved), then s1-s11 (callee-saved)
Call save:            only saves a-regs that hold values LIVE across the call
Prologue:             only saves s-regs that are actually used (already done)
```

Net effect: most temporaries go into a-regs. Short-lived values (not live across
calls) cost ZERO saves. Values live across calls pay a save at that specific
call site (not at every call). s-regs are used only for values that survive
multiple calls — rare in practice.

## Implementation Plan

### Phase 1: Record liveness at call sites during interference graph build

**File**: `src/reg_alloc/reg_alloc.cpp` — `Visit(IRFunctionNode*)`

During the reverse block walk, when we encounter a call instruction, snapshot
the current `live` BitSet BEFORE processing the call's defs/uses. Store these
in a new data structure:

```cpp
// Map: IRCallInstructionNode* → BitSet of live promotable variables at that call
std::vector<std::pair<IRCallInstructionNode*, BitSet>> call_live_sets_;
```

**Code change** (~10 lines): In the reverse walk loop, after collecting move
pairs and before processing defs/uses, check `dynamic_cast<IRCallInstructionNode*>(ins)`.
If it's a call, clone the current `live` set and append to `call_live_sets_`.

### Phase 2: Compute per-call dead a-reg masks after coloring

**File**: `src/reg_alloc/reg_alloc.cpp` — `Visit(IRFunctionNode*)`

After `ig.Color()` returns, iterate `call_live_sets_`. For each entry:
1. Get the live set at the call
2. For each a-reg (phys reg IDs 10-17), check: does any live variable map to
   this physical register?
3. If NO live variable uses this a-reg → it's dead at this call → set bit in mask
4. Store the dead mask on the IRCallInstructionNode

```cpp
// New field in IRCallInstructionNode (IR/IR_node.h):
uint32_t dead_a_regs_mask_ = 0;  // bit i = a-reg (10+i) is dead at this call

// Logic after coloring:
for (auto &[call, live] : call_live_sets_) {
    uint32_t dead_mask = 0;
    for (uint32_t a = 0; a < 8; ++a) {
        uint32_t phys_reg = 10 + a;
        bool live_found = false;
        live.ForEach([&](uint32_t var_id) {
            if (ig.HasPhysReg(var_id) && ig.GetPhysReg(var_id) == phys_reg)
                live_found = true;
        });
        if (!live_found) dead_mask |= (1u << a);
    }
    call->dead_a_regs_mask_ = dead_mask;
}
```

### Phase 3: Modify SaveRegister to respect dead masks

**File**: `src/IR_visitor/assembly_generator/assembly_generator.cpp`

In `Visit(IRCallInstructionNode)`, before calling `SaveRegister()`, mark dead
a-regs as invalid so they're skipped:

```cpp
void AssemblyGenerator::Visit(IRCallInstructionNode *node) {
    // ... (existing builtin_memcpy inline logic) ...

    // Mark dead a-regs as "don't save" before SaveRegister
    uint32_t dead_mask = node->dead_a_regs_mask_;
    for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
        if (dead_mask & (1u << i)) {
            a_reg_valid_[i] = false;  // pretend it's already saved
        }
    }

    SaveRegister();  // only saves valid (live) a-regs
    // ... rest of call handling ...
}
```

**Key invariant**: After marking dead a-regs as valid=false, `SaveRegister`
skips them. `FlushSavedRegisters` would restore them (wasteful but harmless —
dead a-regs are never read). If a dead a-reg later becomes live (defined), its
valid flag is set to true by the defining instruction.

### Phase 4: Flip the color pool

**File**: `src/reg_alloc/reg_alloc.cpp`

Swap the pool order so a-regs come first:

```cpp
static const std::vector<uint32_t> kColorPool = {
    10, 11, 12, 13, 14, 15, 16, 17,               // a0-a7 (now preferred)
    9, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27     // s1-s11 (fallback)
};
```

With per-call save optimization in place, a-regs are now cheaper than s-regs:
- a-reg dead at all calls → zero overhead
- a-reg live at one call → saved once at that call
- s-reg → saved/restored in prologue/epilogue of entire function

### Phase 5: Optimize FlushSavedRegisters (optional polish)

Currently `FlushSavedRegisters` restores ALL invalid a-regs, including dead ones.
This causes wasted `ld` instructions for dead a-regs. To avoid this:

Track a `saved_a_regs_mask_` per block — OR of all dead masks across all calls
in the block. Only a-regs NOT in this mask were actually saved and need restoring.

```cpp
// In assembly generator block visitor:
uint32_t saved_mask = 0;
// ... accumulate during call visits ...

// In FlushSavedRegisters:
for (uint32_t i = 0; i < current_a_reg_used_; ++i) {
    if (!a_reg_valid_[i] && !(dead_mask & (1u << i))) {
        // Only restore if actually saved (not just dead)
        EmitMem("ld", "a" + std::to_string(i), "sp", ...);
        a_reg_valid_[i] = true;
    }
}
```

## Files Changed

| File | Change | Effort |
|------|--------|--------|
| `IR/IR_node.h` | Add `dead_a_regs_mask_` field to `IRCallInstructionNode` | 2 lines |
| `reg_alloc/reg_alloc.cpp` | Record call live sets, compute dead masks, flip pool | ~50 lines |
| `assembly_generator/assembly_generator.cpp` | Use dead mask before SaveRegister | ~10 lines |
| `assembly_generator/assembly_generator.h` | Optional: `saved_a_regs_mask_` for polish | ~5 lines |

## Testing Strategy

1. **Phase 1-2 first** (dead mask computation, no behavior change): Add debug
   logging to verify masks are computed correctly. All 50 tests must pass.

2. **Phase 3 alone** (use dead masks, keep s-reg-first pool): Verify sd/ld
   counts decrease. All 50 tests must pass.

3. **Phase 4** (flip pool): Verify further sd/ld reduction. All 50 tests must pass.

4. **Phase 5** (polish): Verify minimal wasted restores.

Each phase is independently testable and committable.

## Risk Analysis

- **Correctness**: Dead masks must correctly identify which a-regs are live.
  A false positive (thinking a reg is dead when it's live) → skipped save →
  clobbered value → wrong result or crash.
  Mitigation: conservative — only mark dead if CERTAIN no live variable uses it.

- **Precolored parameters**: Function parameters are precolored to a0-a7.
  A parameter might be dead by the time of a call. The liveness analysis
  (CFG-based) already tracks this. The `live` set at the call correctly
  reflects whether the parameter is still live.

- **Interaction with const cache**: t3/t4 are reloaded after each call.
  No interaction with a-reg save optimization.

- **Interaction with leaf functions**: Leaf functions already use a-reg-first
  pool. No change needed for leaf functions — they already have zero call saves.
