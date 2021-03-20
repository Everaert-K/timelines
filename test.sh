#!/bin/bash

exitcode=0
for entry in tests/input/*
do
    diff <(build/detective tests/input/$(basename $entry)) <(cat tests/expected/$(basename $entry)) >/dev/null
    if [[ $? -ne 0 ]]; then
        exitcode=$((exitcode + 1)) 
        echo "failed test $entry"
    else
        echo "succeeded test $entry"
    fi
done
 
exit $exitcode

