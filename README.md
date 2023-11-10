# fx

## dependencies
- llvm16

## what works
- floating point arithmetic (with order of operations!)
- custom functions
- can be imported as .o file into c project (using extern)
- look at test/test.txt as an example of a working script!!

## cmake
- to configure, run `cmake -B build -G Ninja -DLLVM_TARGETS_TO_BUILD=x86`
- to run test, run `cmake --build build` this builds both executables, `fx` and `testbin`, which uses the files in the test directory. it's not perfect, will be improved.