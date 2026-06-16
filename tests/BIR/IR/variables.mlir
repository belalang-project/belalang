// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

bir.func @main() -> !bir.int {
  %0 = bir.var.declare "x" : !bir.ref<!bir.int>
  %1 = bir.var.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %1 : !bir.int
}
