// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK-LABEL: bir.func @basic
bir.func @basic() {
  // CHECK: %[[C1:.*]] = bir.alloc_stack : !bir.ref<!bir.int>
  %1 = bir.alloc_stack : !bir.ref<!bir.int>
  bir.return
}
