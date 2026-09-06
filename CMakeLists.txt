cmake_minimum_required(VERSION 3.24)

project(Belalang VERSION 0.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BELALANG_BUILD_TESTS "Build unit and lit tests" ON)
option(BELALANG_BUILD_TOOLS "Build developer tools" ON)
set(BELALANG_LLVM_BUILD_DIR "" CACHE PATH
  "Path to an llvm-project build directory containing LLVM and MLIR")

if(NOT BELALANG_LLVM_BUILD_DIR)
  message(FATAL_ERROR "Set BELALANG_LLVM_BUILD_DIR to an llvm-project build directory")
endif()
cmake_path(ABSOLUTE_PATH BELALANG_LLVM_BUILD_DIR NORMALIZE)

set(LLVM_DIR "${BELALANG_LLVM_BUILD_DIR}/lib/cmake/llvm")
set(MLIR_DIR "${BELALANG_LLVM_BUILD_DIR}/lib/cmake/mlir")
if(NOT EXISTS "${LLVM_DIR}/LLVMConfig.cmake" OR
   NOT EXISTS "${MLIR_DIR}/MLIRConfig.cmake")
  message(FATAL_ERROR
    "BELALANG_LLVM_BUILD_DIR must contain lib/cmake/llvm/LLVMConfig.cmake "
    "and lib/cmake/mlir/MLIRConfig.cmake")
endif()

find_package(ZLIB REQUIRED)
find_package(LibXml2 REQUIRED)
find_package(LLVM REQUIRED CONFIG)
find_package(MLIR REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" "${MLIR_CMAKE_DIR}")
message(STATUS "Using LLVM/MLIR build: ${BELALANG_LLVM_BUILD_DIR}")
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")

include(AddLLVM)
include(AddMLIR)
include(TableGen)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

if(NOT LLVM_ENABLE_RTTI)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>")
endif()
if(NOT LLVM_ENABLE_EH)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>")
endif()

include(FetchContent)

find_package(muopt CONFIG QUIET)
if(NOT muopt_FOUND)
  FetchContent_Declare(
    muopt
    GIT_REPOSITORY https://github.com/secona/muopt.git
    GIT_TAG b132583b93025d23871faf237578f266e798315a
    GIT_SHALLOW FALSE
  )
  FetchContent_MakeAvailable(muopt)
endif()

include(CTest)
if(BUILD_TESTING AND BELALANG_BUILD_TESTS)
  find_package(GTest CONFIG QUIET)
  if(NOT GTest_FOUND)
    FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.18.0
    )
    FetchContent_MakeAvailable(googletest)
  endif()
endif()

set(BELALANG_SOURCE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(BELALANG_GENERATED_INCLUDE_DIR "${PROJECT_BINARY_DIR}/include")
file(MAKE_DIRECTORY "${BELALANG_GENERATED_INCLUDE_DIR}/belalang")
include_directories(
  "${BELALANG_SOURCE_INCLUDE_DIR}"
  "${BELALANG_GENERATED_INCLUDE_DIR}"
)

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(brt)
if(BELALANG_BUILD_TOOLS)
  add_subdirectory(tools)
endif()
add_subdirectory(bin)
