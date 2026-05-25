#!/usr/bin/env python3
"""Performance visualization for RCompiler.

To add a new version: drop a new N.txt file (e.g. 3.txt) and add an entry
to VERSION_LABELS below.
"""

import re
import os
import sys
from pathlib import Path
from collections import defaultdict

import matplotlib.pyplot as plt
import matplotlib.ticker as ticker
import numpy as np

# ── Maintainer: add new versions here ──────────────────────────────────────
VERSION_LABELS = {
    "1": "origin",
    "2": "origin + mem2reg",
    "3": "origin + mem2reg + trivial graph coloring (no coalescing)",
}
# ───────────────────────────────────────────────────────────────────────────

PERF_DIR = Path(__file__).resolve().parent
OUT_DIR = PERF_DIR / "plots"
OUT_DIR.mkdir(exist_ok=True)


def parse_file(filepath: Path) -> list[int]:
    """Extract Total cycles from a performance log file."""
    text = filepath.read_text(encoding="utf-8", errors="replace")
    # Strip ANSI escape codes
    text = re.sub(r"\x1b\[[0-9;]*m", "", text)
    # Remove null bytes
    text = text.replace("\x00", "")
    cycles = [int(m) for m in re.findall(r"^Total cycles:\s*(\d+)", text, re.MULTILINE)]
    if not cycles:
        print(f"Warning: no data found in {filepath}", file=sys.stderr)
    return cycles


def load_all_versions() -> dict[str, list[int]]:
    """Load all version files from the performance directory."""
    versions = {}
    for f in sorted(PERF_DIR.glob("[0-9]*.txt")):
        ver = f.stem  # "1", "2", ...
        if ver in VERSION_LABELS:
            versions[ver] = parse_file(f)
    return versions


def speedup_chart(versions: dict[str, list[int]]):
    """Bar chart of speedup vs origin (v1) for each test case, sorted."""
    if "1" not in versions:
        print("No baseline (1.txt) found, skipping speedup chart", file=sys.stderr)
        return

    baseline = np.array(versions["1"], dtype=np.float64)
    n = len(baseline)

    fig, axes = plt.subplots(1, len(versions) - 1, figsize=(max(6, 4 * (len(versions) - 1)), 7),
                             squeeze=False)
    fig.suptitle("Speedup vs origin (higher = better)", fontsize=13, fontweight="bold")

    for idx, (ver, label) in enumerate([(v, VERSION_LABELS[v]) for v in versions if v != "1"]):
        ax = axes[0][idx]
        data = np.array(versions[ver], dtype=np.float64)
        if len(data) != n:
            print(f"Test count mismatch: origin={n}, {ver}={len(data)}", file=sys.stderr)
            continue

        ratio = baseline / data
        # Sort by ratio
        order = np.argsort(ratio)
        colors = ["#2ca02c" if r >= 1.0 else "#d62728" for r in ratio[order]]
        x = np.arange(n)
        ax.bar(x, ratio[order], color=colors, width=0.8)
        ax.axhline(y=1.0, color="gray", linestyle="--", linewidth=0.8)
        ax.set_title(f"{label}  (geomean {np.exp(np.log(ratio).mean()):.2f}x)", fontsize=11)
        ax.set_ylabel("Speedup ratio")
        ax.set_xlabel("Test case (sorted)")
        ax.set_ylim(0, max(ratio.max() * 1.1, 2.0))
        ax.yaxis.set_major_formatter(ticker.FormatStrFormatter("%.1fx"))

    plt.tight_layout()
    out = OUT_DIR / "speedup.png"
    fig.savefig(out, dpi=150, bbox_inches="tight")
    plt.close(fig)
    print(f"Saved {out}")


def cycles_comparison(versions: dict[str, list[int]], max_tests_per_chart: int = 25):
    """Grid of bar charts comparing Total cycles across versions, split into
    pages if needed. Log scale because cycle counts span many orders of
    magnitude."""
    n_tests = len(next(iter(versions.values())))
    n_vers = len(versions)
    bar_width = 0.8 / n_vers
    vers_list = sorted(versions.keys())

    for page_start in range(0, n_tests, max_tests_per_chart):
        page_end = min(page_start + max_tests_per_chart, n_tests)
        n = page_end - page_start
        fig, ax = plt.subplots(figsize=(max(12, n * 0.35), 6))
        x = np.arange(n)

        for i, ver in enumerate(vers_list):
            data = np.array(versions[ver][page_start:page_end], dtype=np.float64)
            offset = (i - (n_vers - 1) / 2) * bar_width
            ax.bar(x + offset, data, bar_width, label=VERSION_LABELS.get(ver, ver),
                   alpha=0.85)

        ax.set_yscale("log")
        ax.set_ylabel("Total cycles (log scale)")
        ax.set_xlabel("Test case index")
        ax.set_title(f"Total cycles: version comparison  (tests {page_start}–{page_end - 1})")
        ax.set_xticks(x)
        ax.set_xticklabels([str(i + page_start) for i in range(n)], rotation=90, fontsize=7)
        ax.legend()
        ax.yaxis.set_major_formatter(ticker.ScalarFormatter())

        plt.tight_layout()
        suffix = f"_{page_start}_{page_end - 1}" if n_tests > max_tests_per_chart else ""
        out = OUT_DIR / f"cycles_compare{suffix}.png"
        fig.savefig(out, dpi=150, bbox_inches="tight")
        plt.close(fig)
        print(f"Saved {out}")


def summary_table(versions: dict[str, list[int]]):
    """Print a text summary table to stdout."""
    print("\n" + "=" * 70)
    print(f"{'Version':<12} {'Total cycles':>18} {'Geomean vs origin':>20}")
    print("-" * 70)

    baseline = np.array(versions.get("1", []), dtype=np.float64)
    for ver in sorted(versions.keys()):
        data = np.array(versions[ver])
        total = data.sum()
        label = VERSION_LABELS.get(ver, ver)
        if len(baseline) == len(data) and ver != "1":
            ratio = np.exp(np.log(baseline / data).mean())
            print(f"{label:<12} {total:>18,} {ratio:>19.2f}x")
        elif ver == "1":
            print(f"{label:<12} {total:>18,}  {'(baseline)':>19}")
        else:
            print(f"{label:<12} {total:>18,}  {'(N/A - test count mismatch)':>19}")
    print("=" * 70)


def main():
    versions = load_all_versions()
    if not versions:
        print("No version data found in", PERF_DIR, file=sys.stderr)
        sys.exit(1)

    # Validate test counts match
    counts = {v: len(d) for v, d in versions.items()}
    if len(set(counts.values())) != 1:
        print("Warning: version test counts differ:", counts, file=sys.stderr)

    summary_table(versions)
    speedup_chart(versions)
    cycles_comparison(versions)
    print("\nDone. Plots saved to", OUT_DIR)


if __name__ == "__main__":
    main()
