cmake_minimum_required(VERSION 3.21)
project(cfire C)

set(CMAKE_C_STANDARD 11)

add_executable(cfire_test test.c)

add_library(cfire STATIC cfire.c)

install(
    TARGETS cfire
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(
    FILES cfire.h
    DESTINATION include
)
