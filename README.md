# fx
writing a language in Rust, then extending it to C for LLVM

# rust-c-ffi
use cbindgen (with cbindgen.toml)

# todo
- look into using doubly linked lists instead of vectors for the parser/lexer tokens
- predicate parser
- interface between rust/c++

# tests
- see performance of ffi interface (C++ side) of not using std::move vs. using std::move