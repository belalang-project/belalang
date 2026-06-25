{
  description = "The Belalang Programming Language";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-parts.url = "github:hercules-ci/flake-parts";
  };

  outputs =
    inputs:
    inputs.flake-parts.lib.mkFlake { inherit inputs; } {
      systems = [ "x86_64-linux" ];

      perSystem =
        {
          config,
          pkgs,
          system,
          ...
        }:
        {
          devShells.default = (pkgs.buildFHSEnv {
            name = "bazel";
            targetPkgs = pkgs: [
              pkgs.bazelisk
              pkgs.zlib
              pkgs.python313
              pkgs.clang-tools
              pkgs.just
            ];
            profile = ''
              export BRT_DIR="$PWD/bazel-bin/brt"
            '';
          }).env;
        };
    };
}
