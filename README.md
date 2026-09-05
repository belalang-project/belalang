# Belalang 🦗

> [!NOTE]
> Belalang is currently highly experimental and in very early development. The language design and syntax are subject to change.

Belalang (Indonesian word for Grasshopper) is an experimental compiled language built using MLIR and LLVM.
Inspired by ClangIR, it utilizes a custom MLIR dialect for its lowering system
and uses a Garbage Collector under the hood for automatic memory management.

## Examples

Examples of Belalang programs can be found in the [examples](./examples)
directory.

## Building from Source

Belalang uses [Bazel](https://bazel.build/) as its build system. For detailed
instructions on how to set up and build the project, please refer to the
[documentation](./docs).

An in-progress CMake build is also available alongside Bazel. Point it at the
root of a local `llvm-project` checkout (the directory containing `llvm/` and
`mlir/`):

```sh
cmake -S . -B build -G Ninja \
  -DBELALANG_LLVM_SOURCE_DIR=/path/to/llvm-project \
  -DLLVM_TARGETS_TO_BUILD=X86
cmake --build build
ctest --test-dir build
```

If LLVM and MLIR are already installed as CMake packages,
`BELALANG_LLVM_SOURCE_DIR` may be omitted and `LLVM_DIR`/`MLIR_DIR` supplied
instead. Unit tests and developer tools can be disabled with
`BELALANG_BUILD_TESTS=OFF` and `BELALANG_BUILD_TOOLS=OFF`, respectively.

## License

Licensed under either of

- Apache License, Version 2.0 ([LICENSE-APACHE](LICENSE-APACHE) or
  <http://www.apache.org/licenses/LICENSE-2.0>)
- MIT license ([LICENSE-MIT](LICENSE-MIT) or
  <http://opensource.org/licenses/MIT>)

at your option.

## Contribution

Check our [GitHub Issues](https://github.com/belalang-project/belalang/issues)
tab to see what is currently being discussed or worked on.

Unless you explicitly state otherwise, any contribution intentionally submitted
for inclusion in the work by you, as defined in the Apache-2.0 license, shall be
dual licensed as above, without any additional terms or conditions.
