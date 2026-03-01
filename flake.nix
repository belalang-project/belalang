{
  description = "The Belalang Programming Language";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";

    crane.url = "github:ipetkov/crane";

    rust-overlay.url = "github:oxalica/rust-overlay";
    rust-overlay.inputs.nixpkgs.follows = "nixpkgs";

    treefmt-nix.url = "github:numtide/treefmt-nix";
    treefmt-nix.inputs.nixpkgs.follows = "nixpkgs";

    git-hooks-nix.url = "github:cachix/git-hooks.nix";
    git-hooks-nix.inputs.nixpkgs.follows = "nixpkgs";
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" ];

      imports = [
        inputs.git-hooks-nix.flakeModule
        inputs.treefmt-nix.flakeModule
      ];

      perSystem =
        {
          config,
          pkgs,
          system,
          ...
        }:
        let
          rust-toolchain = pkgs.rust-bin.fromRustupToolchainFile ./rust-toolchain.toml;
          craneLib = (inputs.crane.mkLib pkgs).overrideToolchain rust-toolchain;

          # Source of the entire workspace
          src = craneLib.cleanCargoSource ./.;

          # Build cargo deps of the entire workspace.
          cargoArtifacts = craneLib.buildDepsOnly { inherit src; };

          pname = "belalang";

          belalang = craneLib.buildPackage {
            inherit pname src cargoArtifacts;
          };

          llvm = pkgs.llvmPackages_latest;
          stdenv = llvm.libcxxStdenv;

          belalang-cpp = stdenv.mkDerivation {
            name = "belalang";

            src = ./.;

            buildInputs = [
              pkgs.cli11
              pkgs.gtest
            ];

            nativeBuildInputs = [
              pkgs.cmake
              pkgs.ninja
              pkgs.pkg-config
            ];

            cmakeFlags = [ "-GNinja" ];

            buildPhase = ''
              cmake --build . --target belalang
            '';

            installPhase = ''
              mkdir -p $out/bin
              cp bin/belalang $out/bin/
            '';
          };
        in
        {
          _module.args = {
            pkgs = import inputs.nixpkgs {
              inherit system;
              overlays = [ inputs.rust-overlay.overlays.default ];
            };
          };

          packages.default = belalang;
          packages.belalang-cpp = belalang-cpp;

          checks = {
            workspace-test = craneLib.cargoNextest {
              inherit pname src cargoArtifacts;
              cargoNextestExtraArgs = "--workspace --all-features";
            };

            workspace-clippy = craneLib.cargoClippy {
              inherit pname src cargoArtifacts;
              cargoClippyExtraArgs = "--workspace --all-targets --all-features --keep-going -- -D warnings";
            };

            workspace-build = craneLib.cargoBuild {
              inherit pname src cargoArtifacts;
              cargoExtraArgs = "--workspace";
            };
          };

          devShells.default = pkgs.mkShell.override { inherit stdenv; } {
            name = "belalang";
            buildInputs = [
              rust-toolchain
              pkgs.cargo-nextest
            ];
            inputsFrom = [ belalang-cpp ];
            shellHook = ''
              ${config.pre-commit.installationScript}

              INCLUDES=$(echo | clang++ -v -x c++ - 2>&1 | grep "^ /" | sed 's/^ /    - "-I/' | sed 's/$/"/')

              cat > .clangd << EOF
              CompileFlags:
                Add:
              $INCLUDES
                CompilationDatabase: build/
              EOF
            '';
          };

          pre-commit = {
            check.enable = true;
            settings.hooks = {
              check-merge-conflicts.enable = true;
            };
          };

          treefmt.programs = {
            nixfmt.enable = true;
            rustfmt.enable = true;
            rustfmt.package = rust-toolchain;
            taplo.enable = true;
            yamlfmt.enable = true;
            keep-sorted.enable = true;

            cmake-format.enable = true;
            clang-format.enable = true;
          };
        };
    };
}
