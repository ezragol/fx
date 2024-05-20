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

llvm_map_components_to_libnames(llvm_libs core passes nativecodegen asmparser x86asmparser)

target_link_libraries(lib codegen ${llvm_libs} ${LIB_FX_PATH})