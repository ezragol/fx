#!/bin/bash
cargo build
cmake build -B build
cd build
ninja
cd ..