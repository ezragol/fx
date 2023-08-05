#!/bin/bash

function fx_echo() {
    echo
    echo "$1"
}

function build_test_exec {
    fx_echo "-> building test objects"
    if llc -filetype=obj test.bc ;  then
        fx_echo "<- building test executable"
        if clang -o test test.c test.o ; then
            fx_echo "<- done"
            exit 0
        fi
    fi
}

echo "-> creating test"
if ./build.sh ; then
    fx_echo "-> entering test folder"
    cd test
    echo "-> compiling test object"
    if [ "$1" = "valgrind" ] ; then
        CONTINUE=$(valgrind --tool=memcheck --leak-check=full ../build/fx)
    else
        CONTINUE=$(../build/fx)
    fi
    if [ CONTINUE ] ; then
        build_test_exec
    fi
fi

fx_echo "<- exiting"
exit 1
