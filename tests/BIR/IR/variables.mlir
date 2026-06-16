// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

bir.func @main() -> !bir.int {
  %0 = bir.var.declare "x" : !bir.int
  bir.return %0 : !bir.int
}
