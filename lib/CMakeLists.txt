add_subdirectory(codegen)

add_library(lib)

target_sources(lib
  PUBLIC
    compiler.cpp
    compiler.h
    errors.cpp
    errors.h
    interface.cpp
    interface.h)

target_link_libraries(lib codegen)