#!/bin/bash

if [ $# -ne 1 ]; then
    echo "expect index"
    exit 1
fi

INDEX="$1"
clang-15 -S -c --target=riscv32-unknown-elf -O0 "tmp_data/${INDEX}.ll" -o "tmp_data/${INDEX}.s"

sed -i 's/@plt//g' "tmp_data/${INDEX}.s"