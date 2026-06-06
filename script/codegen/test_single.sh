#!/bin/bash

if [ $# -ne 1 ]; then
    echo "expect index"
    exit 1
fi

INDEX="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
SYSROOT="$PROJECT_DIR/tools/riscv64-linux/sysroot"
GCC="riscv64-linux-gnu-gcc-12"

cmake --build cmake-build-debug
./cmake-build-debug/codegen_single < "tmp_data/codegen/${INDEX}.rx" > "tmp_data/codegen/${INDEX}_optimized.s"

$GCC -march=rv64gc -mabi=lp64d \
  --sysroot="$SYSROOT" \
  -static \
  "tmp_data/codegen/${INDEX}_optimized.s" -o "tmp_data/codegen/${INDEX}_optimized"

qemu-riscv64 "tmp_data/codegen/${INDEX}_optimized" < "tmp_data/codegen/${INDEX}.in" > "tmp_data/codegen/${INDEX}.out"