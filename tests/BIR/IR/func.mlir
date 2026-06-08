// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK:      module {
// CHECK-NEXT:   bir.func {
// CHECK-NEXT:     %0 = bir.constant 1 : !bir.int
// CHECK-NEXT:     bir.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func {
  %0 = bir.constant 1 : !bir.int
  bir.return
}
