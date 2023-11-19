cmake_minimum_required(VERSION 3.22)
project(fx)

set(CMAKE_CXX_STANDARD 17)

option(src "source file" OFF)
option(exec "executable name" OFF)

if (src)
        set(SRCROOT ${src})
else()
        set(SRCROOT "./example")
endif()

if (exec)
        set(EXEC ${exec})
else()
        set(EXEC "out")
endif()

set(FULLROOT ${CMAKE_SOURCE_DIR}/${SRCROOT})

find_package(LLVM REQUIRED CONFIG)
set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD})

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_executable(fx main.cpp)
add_subdirectory(lib)
add_custom_command(
        OUTPUT ${EXEC}__.o
        DEPENDS ${FULLROOT}.txt
        COMMAND cargo build
        COMMAND fx ${FULLROOT}.txt ${FULLROOT}__.bc
        COMMAND llc -filetype=obj -o ${EXEC}__.o ${FULLROOT}__.bc
)

add_executable(${EXEC}
        out.c
        ${EXEC}__.o)

target_link_libraries(fx PUBLIC lib)

execute_process(COMMAND cargo build)