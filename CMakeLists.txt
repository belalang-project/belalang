cmake_minimum_required(VERSION 3.16)

project(
  belalang
  VERSION 0.1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

add_library(belalang_lib src/Lexer/Lexer.cpp)
target_include_directories(belalang_lib PUBLIC include)

add_subdirectory(bin)

install(TARGETS belalang DESTINATION bin)

add_subdirectory(tests)
