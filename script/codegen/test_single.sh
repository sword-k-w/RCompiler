#!/bin/bash

if [ $# -ne 1 ]; then
    echo "expect index"
    exit 1
fi

INDEX="$1"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
TOOLS_DIR="$PROJECT_DIR/tools/riscv64-linux"
SYSROOT="$TOOLS_DIR/sysroot"
LLD_BIN="$TOOLS_DIR/lld/bin"

cmake --build cmake-build-debug
./cmake-build-debug/codegen_single < "tmp_data/codegen/${INDEX}.rx" > "tmp_data/codegen/${INDEX}_optimized.s"

PATH="$LLD_BIN:$PATH" \
  clang-21 --target=riscv64-linux-gnu -march=rv64gc \
  -fuse-ld=lld-21 \
  --sysroot="$SYSROOT" \
  -B"$SYSROOT/riscv64-linux-gnu/lib" \
  -L"$SYSROOT/riscv64-linux-gnu/lib" \
  -L"$SYSROOT/lib/gcc-cross/riscv64-linux-gnu/12" \
  -static \
  "tmp_data/codegen/${INDEX}_optimized.s" -o "tmp_data/codegen/${INDEX}_optimized"

qemu-riscv64 "tmp_data/codegen/${INDEX}_optimized" < "tmp_data/codegen/${INDEX}.in" > "tmp_data/codegen/${INDEX}.out"