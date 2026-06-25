# Building Belalang

Belalang is a Rust + C++ project built using Bazel. We prefer to use `bazelisk` to automatically use the correct Bazel version specified in `.bazelversion`. Since Bazel hermetically downloads and manages all project dependencies and compiler toolchains (such as Rust and C++), `bazelisk` is the only build tool you need to install on your host system.

## Environment Setup

### Using Nix (Recommended)
If you have Nix installed with flakes enabled, you can enter a shell with the required workspace utilities (including Bazelisk and Just) by running:

```bash
nix develop
```

This shell also automatically configures the `BRT_DIR` environment variable needed for the Belalang Runtime.

### Manual Setup
If you are not using Nix, ensure you have the following installed:
- `bazelisk`
- `just` (optional, for run shortcuts)

You must also set the `BRT_DIR` environment variable to point to the runtime build output:
```bash
export BRT_DIR="$PWD/bazel-bin/brt"
```

## Building and Testing

To build the entire codebase, run:
```bash
bazelisk build //...
```
Or use the shorthand wrapper:
```bash
just build
```

To run all tests:
```bash
bazelisk test //...
```
Or use the shorthand wrapper:
```bash
just test
```
