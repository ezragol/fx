#!/bin/bash

echo "-> creating test"
echo
if ./build.sh ; then
    echo
    echo "-> entering test folder"
    cd test
    echo "-> compiling test object"
    if valgrind --tool=memcheck --leak-check=full ../build/fx ; then
        echo "-> building test executable"
        clang test.c test.o -o test.bc
        echo
        echo "<- done"
    else
        echo
        echo "<- exiting"
        exit 1
    fi
else
    echo
    echo "<- exiting"
    exit 1
fi
