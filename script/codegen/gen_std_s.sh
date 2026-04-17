#!/bin/bash

cmake --build cmake-build-debug
#./cmake-build-debug/IR_test
./cmake-build-debug/codegen_test

for INDEX in {1..50}; do
  echo "compiling ${INDEX}.ll into assembly..."
  clang-21 -S --target=riscv32-unknown-elf -O0 "tmp/${INDEX}.ll" -o "tmp/${INDEX}_std.s"
done