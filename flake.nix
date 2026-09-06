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
            name = "belalang-cmake";
            targetPkgs = pkgs: [
              pkgs.cmake
              pkgs.ninja
              pkgs.clang
              pkgs.clang-tools
              pkgs.lld
              pkgs.git
              pkgs.pkg-config
              pkgs.python313
              pkgs.just
              pkgs.zlib
              pkgs.zlib.dev
              pkgs.libxml2
              pkgs.libxml2.dev
            ];
            profile = ''
              export BRT_DIR="$PWD/build/brt/src"
              export LD_LIBRARY_PATH="${pkgs.lib.makeLibraryPath [
                pkgs.zlib
                pkgs.libxml2
                pkgs.stdenv.cc.cc.lib
              ]}:''${LD_LIBRARY_PATH:-}"
            '';
          }).env;
        };
    };
}
