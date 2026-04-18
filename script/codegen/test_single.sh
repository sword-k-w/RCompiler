#!/bin/bash

if [ $# -ne 1 ]; then
    echo "expect index"
    exit 1
fi

INDEX="$1"

cmake --build cmake-build-debug
./cmake-build-debug/codegen_single < "tmp_data/codegen/${INDEX}.rx" > "tmp_data/codegen/${INDEX}.s"

reimu -f="tmp_data/codegen/${INDEX}.s" -i="tmp_data/codegen/${INDEX}.in" -o "tmp_data/codegen/${INDEX}.out" -s=268000000
