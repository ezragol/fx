# fx

## dependencies
- llvm16

## what works
- floating point arithmetic (with order of operations!)
- custom functions
- can be imported as .o file into c project (using extern)
- look at test/test.txt as an example of a working script!!

## cmake issues
- llvm versions are a little wonky

## build.sh
- all arguments are passed directly to ninja
    ```bash
    ./build.sh -j10
    ```
    this will build with `ninja -j10`

## test.sh
- if you want to run with valgrind, pass with `valgrind`
- if you want to pass valgrind args, use `valgrind*arg*`
- if you want to pass ninja options, use `ninja*arg*`
- for example:
    ```bash
    ./test.sh ninja-j10 valgrind
    ``` 
    this will build with `ninja -j10` and then run the build script with `valgrind`
    ```bash
    ./test.sh ninja-j10 valgrind--suppressions=./suppressions.txt
    ```
    this will build with `ninja -j10` and then run the build script with `valgrind --suppressions=./suppressions.txt`