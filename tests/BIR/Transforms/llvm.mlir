// RUN: %bir-opt --split-input-file --bir-to-llvm %s | %FileCheck %s

// CHECK:      module {
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(0 : i32) : i32
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant 0 : !bir.int
  bir.return
}

// -----

// CHECK:      module {
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(0.000000e+00 : f32) : f32
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant 0.00 : !bir.float
  bir.return
}

// -----

// CHECK:      module {
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(1.230000e+00 : f32) : f32
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant 1.23 : !bir.float
  bir.return
}

// -----

// CHECK: llvm.func @f(i32) -> i32
// CHECK: llvm.func @g(i32)

// CHECK-LABEL: llvm.func @main() -> i32
// CHECK: %[[C1:.*]] = llvm.mlir.constant(1 : i32) : i32
// CHECK: %[[CALL:.*]] = llvm.call @f(%[[C1]]) : (i32) -> i32
// CHECK: llvm.call @f(%[[C1]]) : (i32) -> i32
// CHECK: llvm.return %[[CALL]] : i32

bir.func @f(%arg0 : !bir.int) -> !bir.int
bir.func @g(%arg0 : !bir.int)

bir.func @main() -> !bir.int {
  %0 = bir.constant 1 : !bir.int
  %1 = bir.call @f(%0) : (!bir.int) -> !bir.int
  bir.call @f(%0) : (!bir.int) -> !bir.int
  bir.return %1 : !bir.int
}
