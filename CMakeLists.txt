cmake_minimum_required(VERSION 3.24)

project(Belalang VERSION 0.0.0 LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(BELALANG_BUILD_TESTS "Build Belalang's unit and lit tests" ON)
option(BELALANG_BUILD_TOOLS "Build the Belalang developer tools" ON)
set(BELALANG_LLVM_SOURCE_DIR "" CACHE PATH
  "Path to an llvm-project source checkout (the directory containing llvm/ and mlir/)")

if(BELALANG_LLVM_SOURCE_DIR)
  cmake_path(ABSOLUTE_PATH BELALANG_LLVM_SOURCE_DIR NORMALIZE)
  if(NOT EXISTS "${BELALANG_LLVM_SOURCE_DIR}/llvm/CMakeLists.txt" OR
     NOT EXISTS "${BELALANG_LLVM_SOURCE_DIR}/mlir/CMakeLists.txt")
    message(FATAL_ERROR
      "BELALANG_LLVM_SOURCE_DIR must point to an llvm-project checkout "
      "containing llvm/ and mlir/")
  endif()

  set(LLVM_ENABLE_PROJECTS "mlir" CACHE STRING "LLVM projects to build")
  set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "Build LLVM tests")
  set(LLVM_INCLUDE_EXAMPLES OFF CACHE BOOL "Build LLVM examples")
  set(LLVM_INCLUDE_BENCHMARKS OFF CACHE BOOL "Build LLVM benchmarks")
  set(MLIR_INCLUDE_TESTS OFF CACHE BOOL "Build MLIR tests")
  add_subdirectory("${BELALANG_LLVM_SOURCE_DIR}/llvm"
                   "${PROJECT_BINARY_DIR}/llvm-project")

  list(APPEND CMAKE_MODULE_PATH
    "${BELALANG_LLVM_SOURCE_DIR}/llvm/cmake/modules"
    "${BELALANG_LLVM_SOURCE_DIR}/mlir/cmake/modules"
  )
  message(STATUS "Building against LLVM/MLIR checkout: ${BELALANG_LLVM_SOURCE_DIR}")
else()
  find_package(LLVM REQUIRED CONFIG)
  find_package(MLIR REQUIRED CONFIG)
  list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}" "${MLIR_CMAKE_DIR}")
  message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}: ${LLVM_DIR}")
  message(STATUS "Found MLIR: ${MLIR_DIR}")
endif()

include(AddLLVM)
include(AddMLIR)
include(TableGen)

include_directories(SYSTEM ${LLVM_INCLUDE_DIRS} ${MLIR_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT LLVM_ENABLE_RTTI)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>")
endif()
if(NOT LLVM_ENABLE_EH)
  add_compile_options("$<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>")
endif()

include(FetchContent)
FetchContent_Declare(
  muopt
  GIT_REPOSITORY https://github.com/secona/muopt.git
  GIT_TAG b132583b93025d23871faf237578f266e798315a
  GIT_SHALLOW FALSE
)
FetchContent_MakeAvailable(muopt)

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

add_subdirectory(include)
add_subdirectory(lib)
add_subdirectory(brt)
if(BELALANG_BUILD_TOOLS)
  add_subdirectory(tools)
endif()
add_subdirectory(bin)
