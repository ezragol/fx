cmake_minimum_required(VERSION 3.22)
project(fx)

set(CMAKE_CXX_STANDARD 17)

find_package(LLVM REQUIRED CONFIG)
set(LIB_FX_PATH "${PROJECT_SOURCE_DIR}/target/debug/librfx.a")
set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD})

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(fx 
        cpp/main.cpp
        cpp/compiler.cpp
        cpp/interface.cpp
        cpp/codegen.cpp)

llvm_map_components_to_libnames(llvm_libs 
        support
        core 
        executionengine
        interpreter
        mc
        nativecodegen
        x86asmparser
        passes)

target_link_libraries(fx PUBLIC ${llvm_libs} ${LIB_FX_PATH})

target_include_directories(fx PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/target")