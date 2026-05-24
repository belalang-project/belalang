cmake_minimum_required(VERSION 3.15.0)
project(BelalangIR LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MLIR REQUIRED CONFIG)
unset(MLIR_TABLEGEN_EXE)
find_program(MLIR_TABLEGEN_EXE mlir-tblgen REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" "${MLIR_CMAKE_DIR}")

include(TableGen)
include(AddLLVM)
include(AddMLIR)

include_directories(
	${LLVM_INCLUDE_DIRS}
	${MLIR_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR}/include
)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(tools)
