{ pkgs ? import <nixpkgs> {} }:

(pkgs.buildFHSEnv {
 name = "bazel";
 targetPkgs = pkgs: [
   pkgs.bazelisk
   pkgs.glibc
   pkgs.gcc
   pkgs.zlib
   pkgs.python313
   pkgs.clang-tools
   pkgs.lld
 ];
}).env
