#!/bin/bash

echo "-> building all components"
echo

echo "-> building rust components"
if cargo build ; then
    echo "-> configuring c++ components"
    if cmake build -B build -DLLVM_TARGETS_TO_BUILD=x86 ; then
        echo "-> entering build directory"
        cd build
        echo "-> building c++ components"
        if ninja ; then
            echo
            echo "<- done"
        else
            echo "<- exiting"
            exit 1
        fi
    else
        echo "<- exiting"
        exit 1 
    fi
else
    echo "<- exiting"
    exit 1
fi
