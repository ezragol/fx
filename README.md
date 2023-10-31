# fx

## dependencies
- llvm16

## what works
- floating point arithmetic (with order of operations!)
- custom functions
- can be imported as .o file into c project (using extern)

## Cmake Issues
- llvm versions are a little wonky

## build.sh
- all arguments are passed directly to ninja
    ```bash
    ./build.sh -j10
    ```
    this will build with `ninja -j10`

## test.sh
- this is not a very smart script
- if you give it the argument `valgrind` at any point, it will run valgrind
- the first other argument is passed directly to ninja
- the next argument is passed directly to valgrind
- for example:
    ```bash
    ./test.sh -j10 valgrind
    ``` 
    this will build with `ninja -j10` and then run the build script with `valgrind`
    ```bash
    ./test.sh valgrind -j10 --suppressions=./suppressions.txt
    ```
    this will build with `ninja -j10` and then run the build script with `valgrind --suppressions=./suppressions.txt`
- it is not perfect, hopefully be improved in the future