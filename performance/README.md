# Performance

Records of RISC-V simulator cycle counts across compiler versions.

## Versions

Drop a new `N.txt` file when adding a version, then add its label to
`visualize.py` (`VERSION_LABELS` dict). Both files are needed.

| File | Description |
|------|-------------|
| `1.txt` | origin |
| `2.txt` | origin + mem2reg |
| `3.txt` | origin + mem2reg + trivial graph coloring (no coalescing) |

## Visualization

```
python3 performance/visualize.py
```

Output goes to `performance/plots/`:
- `speedup.png` — per-test speedup vs baseline, sorted
- `cycles_compare_*.png` — side-by-side cycle counts (log scale)

## Data format

Each `.txt` file is a dump of simulator output. One test case per block:

```
Total cycles: <int>
Instruction parsed: <int>
Instruction counts:
# simple   = <int>
# mul      = <int>
# div      = <int>
# mem      = <int>
# branch   = <int>
# jump     = <int>
# jalr     = <int>
# libcMem  = <int>
# libcIO   = <int>
# libcOp   = <int>
```

The parser extracts `Total cycles` only. All versions must have the same
number of test cases (same number of `Total cycles` lines).
