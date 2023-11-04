#!/bin/bash

function fx_echo() {
    echo
    echo "$1"
}

function build_test_exec {
    fx_echo ">> building test objects"
    if llc -filetype=obj test/test.bc ;  then
        fx_echo ">> building test executable"
        if clang -o ./testbin ./test/test.c ./test/test.o ; then
            fx_echo "done 0"
            exit 0
        fi
    fi
}

CWD=$(pwd)
VALGRIND=false
echo ">> creating test build"
for arg in $@
do
    if [[ $arg =~ ^ninja(.*)$ ]] ; then
        NINJA_ARGS="$NINJA_ARGS ${BASH_REMATCH[1]}"
    elif [[ $arg =~ ^valgrind(.*)$ ]] ; then
        VALGRIND=true
        VALGRIND_ARGS="$VALGRIND_ARGS ${BASH_REMATCH[1]}"
    else
        BUILD_ARGS="$BUILD_ARGS$arg"
    fi
done

echo ">> ninja: $NINJA_ARGS"
echo ">> valgrind: $VALGRIND $VALGRIND_ARGS"
echo ">> build: $BUILD_ARGS"

BUILD="./build/fx $BUILD_ARGS"

if [ BUILD ] ; then
    cd $CWD
    echo ">> compiling test object"
    if $VALGRIND ; then
        echo ">> running build executable with valgrind"
        CONTINUE=$(valgrind --tool=memcheck --leak-check=full $VALGRIND_ARGS $BUILD)
    else
        echo ">> running build executable"
        CONTINUE=$($BUILD)
    fi
    if [ CONTINUE ] ; then
        build_test_exec
    fi

    fx_echo "fail 1"
    exit 1
fi
