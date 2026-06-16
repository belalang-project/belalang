// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK: bir.func
bir.func @main() -> !bir.int {
  %0 = bir.var.declare "x" : !bir.ref<!bir.int>
  %1 = bir.constant 12 : !bir.int
  bir.var.store %1 to %0 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.var.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %2 : !bir.int
}
