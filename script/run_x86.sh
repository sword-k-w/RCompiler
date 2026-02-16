#!/bin/bash

cmake --build cmake-build-debug
./cmake-build-debug/IR_test

success_count=0

for INDEX in {1..50}; do
  echo "compiling ${INDEX}.ll into executable..."
  clang-21 "tmp/${INDEX}.ll" -o "tmp/${INDEX}.out" -Wl,-z,stack-size=100000000
  if [ $? -eq 0 ]; then
    tmp/${INDEX}.out < "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.in" > "tmp/${INDEX}.ans"
    if [ $? -eq 0 ]; then
      diff "tmp/${INDEX}.ans" "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.out" -Z
      if [ $? -eq 0 ]; then
        echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!${INDEX} success!"
        ((success_count++))
      else
        echo "*********************${INDEX} failed in diff"
      fi
    else
      echo "@@@@@@@@@@@@@@@@@@@@@${INDEX} failed in executing"
    fi
  else
    echo "^^^^^^^^^^^^^^^^^^^^^${INDEX} failed in generating .s"
  fi
  rm "tmp/${INDEX}.out"
done

echo "${success_count} success out of 50"