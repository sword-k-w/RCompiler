#!/bin/bash

success_count=0

for INDEX in {32..32}; do
  echo "compiling ${INDEX}.ll into assembly..."
  clang-21 -S --target=riscv32-unknown-elf -O0 "tmp_data/${INDEX}.ll" -o "tmp_data/${INDEX}.s"
  if [ $? -eq 0 ]; then
    sed -i 's/@plt//g' "tmp_data/${INDEX}.s"
    echo "${INDEX} success!"
    ((success_count++))
  else
    echo "${INDEX} failed!"
  fi
#  rm "tmp_data/${INDEX}.s"
done

echo "${success_count} success out of 50"