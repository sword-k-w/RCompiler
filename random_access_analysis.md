# Random Memory Access Bottleneck Analysis

## Summary

The compiler generates code that runs **~6x slower than GCC -O1** on two hidden test cases:
- `opti_string_automata_suite` — state machine traversal (random array access in tight loops)
- `opti_inmemory_index_query` — in-memory hash table lookups (pointer chasing)

**Five** root causes were identified by compiling microbenchmarks and examining the generated assembly.

According to the TA, both are characterized by: "随机内存访问比较频繁" (frequent random memory access), "循环里打各种随机内存访问" (various random memory accesses in loops), "数组来 simulate 指针" (arrays simulating pointers).

Four root causes were identified by compiling microbenchmarks and examining the generated assembly.

---

## ROOT CAUSE #1 (HIGHEST IMPACT): i32 array elements stored as 8 bytes

### The bug

`src/IR_visitor/preprocessor/preprocessor.cpp:13-14`:
```cpp
if (node->base_type_ == "i32" || node->base_type_ == "ptr") {
    node->align_ = 8;
    node->allocated_size_ = 8;  // ← BUG: i32 should be 4 bytes!
```

`src/codegen/instruction.cpp:67-68`:
```cpp
assert(type == "i32" || type == "ptr");
return std::make_pair("ld", "sd");  // ← BUG: should be lw/sw for i32!
```

### Effect

Every `[i32; N]` array element occupies **8 bytes** instead of **4 bytes**, doubling
the memory footprint. The compiler emits `ld`/`sd` (64-bit load/store) and `slli 3`
(*8 byte stride) for all i32 array accesses.

For random access patterns like automata transition tables and hash tables:
- Each 64-byte cache line holds **8 elements** instead of **16**
- Cache miss rate approximately **doubles** for random access
- Each L1 miss costs ~4 cycles, L2 miss ~10–20 cycles, LLC/~RAM access ~100–200 cycles
- Memory bandwidth is wasted loading 4 unused bytes per element

### Evidence

Compiled `fn sum_array(arr: &[i32; 1000], len: usize) -> i32`:

```asm
# RCompiler output (our compiler):
.L1:
    slli t1, x14, 3       ; ×8 stride for i32 — WRONG
    add x30, x10, t1      ; &arr[i]
    ld x12, 0(x30)        ; 64-bit load for i32 — WRONG
    addw x13, x15, x12    ; sum += arr[i]
    ...

# GCC -O1 output (correct):
.L3:
    lw a4,0(a5)           ; 32-bit load — correct
    addw a0,a4,a0
    addi a5,a5,4          ; +4 byte stride — correct
    bne a5,a3,.L3
```

**Impact estimate**: 2–3× slowdown for random-access workloads.

---

## ROOT CAUSE #2: No pointer-based induction variable optimization

### The problem

For sequential scans like `for i in 0..len { sum += arr[i]; }`, GCC -O1 converts
index-based addressing into a running pointer:

```asm
# GCC -O1: pointer-based induction
    mv a5,a0              ; running ptr = arr
    slli a1,a1,2          ; end = arr + len*4
    add a3,a0,a1
.L3:
    lw a4,0(a5)           ; load *ptr
    addw a0,a4,a0         ; sum += *ptr
    addi a5,a5,4          ; ptr++ (1 instruction)
    bne a5,a3,.L3         ; if ptr != end, loop
```

RCompiler always uses index-based addressing:
```asm
# RCompiler: index-based (no strength reduction)
.L1:
    slli t1, x14, 3       ; i * elem_size  (2 insns for address)
    add x30, x10, t1      ; &arr[i]
    ld x12, 0(x30)        ; load
    addw x13, x15, x12    ; sum += arr[i]
    ...
```

Even after fixing the i32=8 issue, RCompiler still uses `slli + add` (2 insns)
for address computation vs GCC's `addi` (1 insn) for the running pointer.

This matters for the automata test because many automata implementations scan
input sequentially: `while pos < len { ch = input[pos]; pos += 1; state = trans[state][ch]; }`.
The sequential scan of `input[pos]` should use a running pointer.

**Impact estimate**: ~15–20% overhead for sequential access within mixed workloads.

---

## ROOT CAUSE #3: Redundant `mv` instructions from failed register coalescing

### The problem

After mem2reg + phi elimination, loop induction variables and accumulators pass
through Move instructions at the end of each loop body. The register allocator's
Briggs coalescer (`src/reg_alloc/interference_graph.cpp`) should merge these
virtual registers, but it fails for loop-carried values:

```asm
    addw x13, x15, x12    ; new_sum = sum + arr[i]    → writes x13
    addiw x30, x14, 1     ; new_i = i + 1             → writes x30
    mv x14, x30           ; i = new_i (phi move)      → WASTED
    mv x15, x13           ; sum = new_sum (phi move)  → WASTED
```

The block-level liveness granularity creates spurious interference between a phi's
source and destination through the loop back-edge, preventing coalescing. GCC -O1
updates accumulators in place:

```asm
    addw a0,a5,a0         ; sum += *ptr  — writes a0 directly, no mv needed
```

**Impact estimate**: 2 wasted instructions per loop iteration (~20–25% overhead).

---

## ROOT CAUSE #4: Load-use hazard stalls on dependent memory accesses

### The problem

For dependent random accesses like `b[a[idx[i]]]` (common in automata: state =
transition[current_state][input_char]), the compiler emits back-to-back dependent
loads without any independent work between them:

```asm
    ld x17, 0(x30)        ; load idx1
    slli t1, x17, 3       ; ← STALL: x17 not ready for 1–2 cycles
    add x16, x10, t1      ;
    ld x30, 0(x16)        ; load idx2
    slli t1, x30, 3       ; ← STALL: x30 not ready
    add x17, x11, t1      ;
    ld x16, 0(x17)        ; load b[idx2]
    ...                   ; ← STALL
```

On the in-order RISC-V core modeled by qemu-riscv64, each `ld → use` pair within
1–2 instructions causes pipeline bubbles. With 3 dependent loads per iteration
and no scheduling, that's **3–6 stall cycles** on top of the ~11 instruction
loop body, reducing throughput by 30–50%.

GCC -O1 faces the same data dependency (it's fundamental to pointer chasing),
but its instruction scheduler interleaves loop control with memory operations
to partially hide latency.

**Impact estimate**: ~20–40% overhead (compounds with instruction count overhead).

---

## ROOT CAUSE #5: Struct field accesses recompute base address for every field

### The problem

When accessing multiple fields of the same struct element through an array index
(e.g., `pool[idx].key`, `pool[idx].next`, `pool[idx].value`), the compiler
recomputes the base address from scratch for each field:

```asm
.L15:                          ; Check pool[idx].key
    li t1, 24                  ; ┐ struct size constant
    mul t2, t1, x17            ; │ idx * 24
    add x16, x10, t2           ; ┘ base = pool + idx*24  (1st time)
    ld x16, 0(x16)             ; load .key (offset 0)

    ; ... comparison, branch to .L20 if not found ...

.L20:                          ; Load pool[idx].next
    li t1, 24                  ; ┐ RECOMPUTED from scratch!
    mul t2, t1, x17            ; │ idx * 24  (2nd time)
    add x15, x10, t2           ; ┘ base recomputed  (2nd time)
    addi x16, x15, 16          ; load .next (offset 16)

.L18:                          ; Load pool[idx].value
    li t1, 24                  ; ┐ RECOMPUTED from scratch!
    mul t2, t1, x17            ; │ idx * 24  (3rd time)
    add x16, x10, t2           ; ┘ base recomputed  (3rd time)
    addi x15, x16, 8           ; load .value (offset 8)
```

The address `idx * 24 + pool` is computed 3 times (9 instructions) instead of
once (3 instructions). Six instructions are completely wasted per hash table
entry — **tripling the address computation cost**.

The CSE pass (`src/IR_visitor/cse/cse.cpp`) only eliminates redundant GEP/GEPP
instructions within the same basic block. It does NOT track arithmetic
instructions (`mul`, `add`, `li`), so the redundant `li + mul + add` chains
for struct field access are never eliminated.

This is particularly devastating for the `opti_inmemory_index_query` hash table
test, where each bucket chain traversal accesses multiple fields of the same
struct entry.

Compounding this: with i32=8 bytes (Root Cause #1), the struct `HashEntry { key:
i32, value: i32, next: usize }` is **24 bytes** instead of **16 bytes**
(4+4+8). The `li t1, 24` constant materialization and the `mul` are both
unnecessary overhead on top of the already-bloated struct size.

**Impact estimate**: 6 wasted instructions per struct element access (~40–60%
overhead for hash table bucket traversal).

---

## Combined Impact

| Issue | Effect | Est. Slowdown Factor |
|---|---|---|
| \#1: i32 = 8 bytes | 2× cache misses for arrays + 50% larger structs | 2.0–3.0× |
| \#2: No strength reduction | Extra slli+add per sequential access | 1.15–1.20× |
| \#3: Failed coalescing | 2 extra mv per loop iteration | 1.20–1.25× |
| \#4: Load-use stalls | Pipeline bubbles on dependent loads | 1.20–1.40× |
| \#5: CSE misses struct field recomputation | 3× `li+mul+add` per struct element | 1.30–1.60× |
| **Combined** | | **~4.3–9.5×** |

The observed 6× slowdown falls within the predicted range, suggesting most
issues are simultaneously active in the hidden test cases. The automata test
is primarily affected by root causes #1, #3, and #4; the hash table test by
#1, #3, #4, and #5.

---

## Recommended Fix Priority

### Fix 1: Correct i32 element size (highest priority)

**Files to change:**
- `src/IR_visitor/preprocessor/preprocessor.cpp:13-14`: Change i32 `allocated_size_` from 8 to 4, `align_` from 8 to 4
- `src/codegen/instruction.cpp:67-68`: Return `lw`/`sw` for i32, keep `ld`/`sd` for ptr
- `src/IR_visitor/preprocessor/preprocessor.cpp`: Fix struct alignment interleaving (i1 member followed by i32 member should align i32 to 4, not 8)
- Verify GEPP elem_size calculation works with 4-byte i32 (should naturally via `allocated_size_ / length[0]`)

**Risk**: Need to audit all places that assume i32 occupies a full register width
in memory. Sign-extension after `lw` is automatic on RV64, so arithmetic with
`addw` stays correct.

**Expected gain**: ~2× improvement on random-access workloads.

### Fix 2: Improve register coalescing for loop induction variables

**Files to change:**
- `src/reg_alloc/interference_graph.cpp`: Improve liveness granularity for
  phi-related move instructions so the source and destination don't falsely
  interfere through loop back-edges.

**Expected gain**: ~20–25% instruction reduction in tight loops.

### Fix 3: Implement pointer-based induction (strength reduction)

Convert `for i in 0..len { arr[i] }` into running pointers (`ptr += elem_size`),
eliminating `slli + add` per iteration. Already implemented for `[val; N]` loops;
needs generalization to user-written loops.

**Expected gain**: ~15–20% for sequential scan phases.

### Fix 4: Extend CSE to arithmetic instructions for struct access (high priority)

**Files to change:**
- `src/IR_visitor/cse/cse.cpp`: Extend CSE beyond GEP/GEPP to also track `mul`
  and `add` instructions within basic blocks. When the same `li + mul + add`
  sequence computes the same base address for multiple struct field accesses,
  reuse the first result.

Alternatively, the IR generator could emit a GEP for the struct base and then
constant-offset GEPs for each field (instead of arithmetic instructions for
the base), which the existing CSE would then handle.

**Expected gain**: ~40–60% on hash table bucket traversal (eliminates 6 wasted
instructions per struct element access).

### Fix 5: Instruction scheduling (lower priority, high effort)

Reorder instructions to hide load-use latency on in-order cores.

**Expected gain**: ~20–30% on dependent load chains.

---

## Example: How the hash table inner loop should look

### Current RCompiler output (hash table inner loop, ~28 instructions)

```asm
.L15:                          ; Check key match
    li t1, 24                  ; wasted: recomputing base
    mul t2, t1, x17
    add x16, x10, t2
    mv x15, x16                ; wasted: extra mv
    ld x16, 0(x15)             ; load .key
    sub t0, x16, x14
    sltiu x15, t0, 1
    bnez x15, .L18
.L20:                          ; Load next pointer
    li t1, 24                  ; wasted: recomputing base AGAIN
    mul t2, t1, x17
    add x15, x10, t2
    addi x16, x15, 16          ; load .next
    ld x15, 0(x16)
    mv x18, x15
    mv x16, x9
.L19:
    mv x17, x18                ; wasted: phi coalescing failed
    mv x9, x16                 ; wasted: phi coalescing failed
    j .L16
.L18:                          ; Accumulate value
    li t1, 24                  ; wasted: recomputing base a THIRD time
    mul t2, t1, x17
    add x16, x10, t2
    addi x15, x16, 8           ; load .value
    ld x16, 0(x15)
    addw x16, x9, x16
    li x15, 0                  ; set idx=0 to break
    mv x18, x15
    mv x16, x16                ; useless self-mv
    j .L19
```

### After fixing #1, #3, #5 (target: ~18 instructions, ~35% reduction)

```asm
.L15:                          ; Entry point with base pre-computed
    ld x15, 0(x17)             ; load .key (base already in x17)
    sub t0, x15, x14
    sltiu x15, t0, 1
    bnez x15, .L18
.L20:                          ; Load next pointer
    ld x18, 16(x17)            ; load .next (base reused!)
    j .L16
.L18:                          ; Accumulate value
    lw x15, 4(x17)             ; load .value (lw! 4-byte i32, base reused)
    addw x16, x16, x15         ; sum += .value (in place)
    li x18, 0                  ; set idx=0 to break
    j .L16
```

(Plus the outer loop body with running pointer for sequential key array scan.)

---

## Microbenchmark Reproduction

See `test_rand_access.rx`. Compile with:

```bash
./build/code < test_rand_access.rx > tmp/test.s 2> tmp/builtin.s
cat tmp/builtin.s tmp/test.s > tmp/combined.s
riscv64-linux-gnu-gcc-12 -static -march=rv64gc tmp/combined.s -o tmp/test_out
```

Then compare loop bodies against the equivalent C compiled with `riscv64-linux-gnu-gcc-12 -O1 -S`.

### Key comparisons per pattern

**Sequential sum:** RCompiler 8 insns/iter vs GCC 4 insns/iter (2× overhead)

**Random lookup `table[indices[i]]`:** RCompiler 9 insns/iter vs GCC 7 insns/iter
(1.3× overhead, PLUS 2× memory footprint from 8-byte i32)

**Dependent chase `b[a[idx[i]]]`:** RCompiler 11 insns/iter vs GCC ~10 insns/iter
(1.1× overhead, PLUS memory bloat and load-use stalls)
