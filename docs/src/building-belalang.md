# Building Belalang

Belalang is a C++ project built using CMake.

## Environment Setup

### Using Nix (Recommended)
If you have Nix installed with flakes enabled, you can enter a shell with the required workspace utilities by running:

```bash
nix develop
```

This shell also automatically configures the `BRT_DIR` environment variable needed for the Belalang Runtime.

### Manual Setup
If you are not using Nix, install CMake 3.24 or newer, Ninja, a C++17 compiler,
Python, zlib, libxml2, and `just` (optional, for shortcuts). You also need an
LLVM build containing MLIR, `llvm-lit`, `FileCheck`, and `not`.

Point CMake to the LLVM build directory:
```bash
export BELALANG_LLVM_BUILD_DIR=/path/to/llvm-project/build
```

## Building and Testing

To build the entire codebase, run:
```bash
cmake -S . -B build -G Ninja \
  -DBELALANG_LLVM_BUILD_DIR="$BELALANG_LLVM_BUILD_DIR"
cmake --build build
```
Or use the shorthand wrapper:
```bash
just build
```

To run all tests:
```bash
ctest --test-dir build --output-on-failure
```
Or use the shorthand wrapper:
```bash
just test
```
