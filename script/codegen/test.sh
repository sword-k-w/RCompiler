#!/bin/bash

cmake --build cmake-build-debug
success_count=0

for INDEX in {1..50}; do
  ./cmake-build-debug/codegen_single < "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.rx" > "tmp/${INDEX}.s"
  reimu -f="tmp/${INDEX}.s" -i="testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.in" -o "tmp/${INDEX}.out" -s=200000000
  if [ $? -eq 0 ]; then
    diff "tmp/${INDEX}.out" "testcases/IR-1/src/comprehensive${INDEX}/comprehensive${INDEX}.out" -Z
    if [ $? -eq 0 ]; then
      echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!${INDEX} success!"
      ((success_count++))
    else
      echo "*********************${INDEX} failed in diff"
    fi
  else
    echo "@@@@@@@@@@@@@@@@@@@@@${INDEX} failed in executing reimu"
  fi
done