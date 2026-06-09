// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK:      module {
// CHECK-NEXT:   bir.func @main() {
// CHECK-NEXT:     %0 = bir.constant 1 : !bir.int
// CHECK-NEXT:     bir.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant 1 : !bir.int
  bir.return
}

// -----

// CHECK:      module {
// CHECK-NEXT:   bir.func @f(%arg0: !bir.int) {
// CHECK-NEXT:     bir.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @f(%arg0 : !bir.int) {
  bir.return
}

// -----

// CHECK:      module {
// CHECK-NEXT:   bir.func @f(%arg0: !bir.int) -> !bir.int {
// CHECK-NEXT:     bir.return %arg0 : !bir.int
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @f(%arg0 : !bir.int) -> !bir.int {
  bir.return %arg0 : !bir.int
}

// -----

// CHECK:      module {
// CHECK-NEXT:   bir.func @f(%arg0: !bir.int) -> (!bir.int, !bir.int) {
// CHECK-NEXT:     bir.return %arg0, %arg0 : !bir.int, !bir.int
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @f(%arg0 : !bir.int) -> (!bir.int, !bir.int) {
  bir.return %arg0, %arg0 : !bir.int, !bir.int
}
