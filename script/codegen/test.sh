#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
SYSROOT="$PROJECT_DIR/tools/riscv64-linux/sysroot"
GCC="riscv64-linux-gnu-gcc-12"

cmake --build cmake-build-debug
success_count=0

for INDEX in {1..50}; do
  ./cmake-build-debug/codegen_single < "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.rx" > "tmp/${INDEX}_optimized.s"

  $GCC -march=rv64gc -mabi=lp64d \
    --sysroot="$SYSROOT" \
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