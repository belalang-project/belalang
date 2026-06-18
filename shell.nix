{ pkgs ? import <nixpkgs> {} }:

(pkgs.buildFHSEnv {
 name = "bazel";
 targetPkgs = pkgs: [
   pkgs.bazelisk
   pkgs.zlib
   pkgs.python313
   pkgs.clang-tools
   pkgs.just
 ];
 profile = ''
   export BRT_DIR="${toString ./.}/bazel-bin/brt"
 '';
}).env
