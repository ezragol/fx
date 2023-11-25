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
set(FULLROOT_BC ${FULLROOT}__.bc)
set(FULLROOT_TXT ${FULLROOT}.txt)

find_package(LLVM REQUIRED CONFIG)
set(LLVM_LINK_COMPONENTS ${LLVM_TARGETS_TO_BUILD})

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

add_subdirectory(lib)

add_executable(fx main.cpp)

add_custom_target(
        lib_rfx
        COMMAND cargo build)
add_dependencies(fx lib_rfx)
target_link_libraries(fx PUBLIC lib)

add_executable(${EXEC} out.c)
add_library(bytecode STATIC IMPORTED GLOBAL)
add_custom_command(
        OUTPUT ${FULLROOT_BC}
        DEPENDS ${FULLROOT_TXT}
        COMMAND fx ${FULLROOT_TXT} ${FULLROOT_BC}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_custom_target(bytecode_target
        DEPENDS ${FULLROOT_BC})

add_dependencies(bytecode bytecode_target)
set_target_properties(bytecode
    PROPERTIES
    IMPORTED_LOCATION ${FULLROOT_BC})

target_link_libraries(${EXEC} PUBLIC bytecode)