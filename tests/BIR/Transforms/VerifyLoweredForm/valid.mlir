// RUN: %bir-opt --split-input-file --bir-verify-lowered-form %s | %FileCheck %s

// CHECK-LABEL: bir.func @lowered
bir.func @lowered() -> !bir.int {
  %0 = bir.constant #bir.int<42> : !bir.int
  %1 = bir.alloc_stack : !bir.ref<!bir.int>
  bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %2 : !bir.int
}
