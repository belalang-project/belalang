// RUN: %bir-opt --split-input-file --bir-to-llvm %s | %FileCheck %s

// CHECK:      module {
// CHECK-NEXT:   bir.func @main() {
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(0 : i32) : i32
// CHECK-NEXT:     bir.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant 0 : !bir.int
  bir.return
}

// -----

// CHECK:      module {
// CHECK-NEXT:   bir.func @main() {
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(0.000000e+00 : f32) : f32
// CHECK-NEXT:     bir.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant 0.00 : !bir.float
  bir.return
}

// -----

// CHECK:      module {
// CHECK-NEXT:   bir.func @main() {
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(1.230000e+00 : f32) : f32
// CHECK-NEXT:     bir.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant 1.23 : !bir.float
  bir.return
}

