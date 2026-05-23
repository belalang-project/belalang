cmake_minimum_required(VERSION 3.15.0)
project(BelalangIR LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MLIR REQUIRED CONFIG)
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS})

add_library(test STATIC lib/belalang_ir/test.cpp)

target_include_directories(test PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
