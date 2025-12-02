#!/bin/bash

if [ $# -ne 1 ]; then
    echo "expect index"
    exit 1
fi

INDEX="$1"
reimu -f="tmp_data/${INDEX}.s" -i="tmp_data/${INDEX}.in" -o "tmp_data/${INDEX}.out" -s=200000000