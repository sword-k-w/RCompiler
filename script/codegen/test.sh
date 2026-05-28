#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
TOOLS_DIR="$PROJECT_DIR/tools/riscv64-linux"
SYSROOT="$TOOLS_DIR/sysroot"
LLD_BIN="$TOOLS_DIR/lld/bin"

cmake --build cmake-build-debug
success_count=0

for INDEX in {1..50}; do
  ./cmake-build-debug/codegen_single < "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.rx" > "tmp/${INDEX}_optimized.s"

  PATH="$LLD_BIN:$PATH" \
    clang-21 --target=riscv64-linux-gnu -march=rv64gc \
    -fuse-ld=lld-21 \
    --sysroot="$SYSROOT" \
    -B"$SYSROOT/riscv64-linux-gnu/lib" \
    -L"$SYSROOT/riscv64-linux-gnu/lib" \
    -L"$SYSROOT/lib/gcc-cross/riscv64-linux-gnu/12" \
    -static \
    "tmp/${INDEX}_optimized.s" -o "tmp/${INDEX}_optimized"

  if [ $? -eq 0 ]; then
    qemu-riscv64 "tmp/${INDEX}_optimized" < "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.in" > "tmp/${INDEX}.out"
    if [ $? -eq 0 ]; then
      diff "tmp/${INDEX}.out" "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.out" -Z
      if [ $? -eq 0 ]; then
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!${INDEX} success!"
        ((success_count++))
      else
        echo "*********************${INDEX} failed in diff"
      fi
    else
      echo "@@@@@@@@@@@@@@@@@@@@@${INDEX} failed in executing qemu"
    fi
  else
    echo "@@@@@@@@@@@@@@@@@@@@@${INDEX} failed in linking"
  fi
done

echo "${success_count} success out of 50"