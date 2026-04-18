#!/bin/bash

if [ $# -ne 1 ]; then
    echo "expect index"
    exit 1
fi

INDEX="$1"

cmake --build cmake-build-debug
./cmake-build-debug/IR_single < "tmp_data/codegen/${INDEX}.rx" > "tmp_data/codegen/${INDEX}.ll"
clang-21 -S --target=riscv32-unknown-elf -O0 "tmp_data/codegen/${INDEX}.ll" -o "tmp_data/codegen/${INDEX}_std.s"
