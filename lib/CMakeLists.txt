set(LIB_FX_PATH "${PROJECT_SOURCE_DIR}/target/debug/librfx.a")

add_subdirectory(codegen)

add_library(lib)

target_sources(lib
  PUBLIC
    compiler.cpp
    compiler.h
    interface.cpp
    interface.h)

target_include_directories(lib PUBLIC
  "${PROJECT_BINARY_DIR}"
  "${PROJECT_SOURCE_DIR}/target")

target_link_libraries(lib codegen LLVM-${LLVM_VERSION_MAJOR} ${LIB_FX_PATH})