#!/bin/bash
cargo build
cmake build -B build -DLLVM_TARGETS_TO_BUILD=x86
cd build
ninja
cd ..