#!/bin/bash

# Test script using codegen_submit (matches OJ submission pipeline).
# codegen_submit outputs builtins to stderr and assembly to stdout.
# This script merges them so the result can be compiled and run with qemu.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
SYSROOT="$PROJECT_DIR/tools/riscv64-linux/sysroot"
GCC="riscv64-linux-gnu-gcc-12"

cmake --build cmake-build-debug
success_count=0

for INDEX in {1..50}; do
  # codegen_submit: builtins on stderr, asm on stdout
  ./cmake-build-debug/code \
    < "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.rx" \
    > "tmp/${INDEX}_submit_asm.s" \
    2> "tmp/${INDEX}_submit_builtin.s"

  # Merge: builtins first, then asm
  cat "tmp/${INDEX}_submit_builtin.s" "tmp/${INDEX}_submit_asm.s" > "tmp/${INDEX}_submit.s"

  $GCC -march=rv64gc -mabi=lp64d \
    --sysroot="$SYSROOT" \
    -static \
    "tmp/${INDEX}_submit.s" -o "tmp/${INDEX}_submit"

  if [ $? -eq 0 ]; then
    qemu-riscv64 "tmp/${INDEX}_submit" \
      < "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.in" \
      > "tmp/${INDEX}_submit.out"
    if [ $? -eq 0 ]; then
      diff "tmp/${INDEX}_submit.out" \
        "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.out" -Z
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
