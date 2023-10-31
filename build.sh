#!/bin/bash

function fx_echo() {
    echo
    echo "$1"
}

echo ">> building all components"

fx_echo ">> building rust components"
if cargo build ; then
    fx_echo ">> configuring c++ components"
    mkdir -p build
    if cmake -G Ninja -B build -DLLVM_TARGETS_TO_BUILD=x86 ; then
        fx_echo ">> entering build directory"
        cd build
        echo ">> building c++ components"
        if ninja $@ ; then
            fx_echo "done 0"
            exit 0
        fi
    fi
fi

fx_echo "fail 1"
exit 1