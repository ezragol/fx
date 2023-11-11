# fx

## dependencies
- llvm16

## what works
- floating point arithmetic (with order of operations!)
- custom functions
- can be imported as .o file into c project (using extern)
- look at test/test.txt as an example of a working script!!

## cmake
- to configure, run `cmake -B build -G Ninja -DLLVM_TARGETS_TO_BUILD=` (target)
- to build, run `cmake --build build` this builds both executables, `fx` and `out`
- the `out` exec will build the file `test.txt` and then when you you run it, it uses the file in `out.c`
- both the executable name and source file are cmake options that can be set with `-Dsrc` and `-Dexec`