// RUN: %bir-opt --split-input-file --bir-to-llvm %s | %FileCheck %s

// CHECK:      module {
// CHECK-NEXT:   llvm.func @brt_mmtk_init()
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     llvm.call @brt_mmtk_init() : () -> ()
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
// CHECK-NEXT:   llvm.func @brt_mmtk_init()
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     llvm.call @brt_mmtk_init() : () -> ()
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
// CHECK-NEXT:   llvm.func @brt_mmtk_init()
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     llvm.call @brt_mmtk_init() : () -> ()
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(1.230000e+00 : f32) : f32
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant 1.23 : !bir.float
  bir.return
}

// -----

// CHECK: llvm.func @brt_mmtk_init()
// CHECK: llvm.func @f(i32) -> i32
// CHECK: llvm.func @g(i32)

// CHECK-LABEL: llvm.func @main() -> i32
// CHECK: llvm.call @brt_mmtk_init() : () -> ()
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

// -----

// CHECK: llvm.func @brt_mmtk_init()
// CHECK: llvm.func @brt_mmtk_alloc(i64) -> !llvm.ptr

// CHECK-LABEL: llvm.func @main() -> i32
// CHECK: llvm.call @brt_mmtk_init() : () -> ()
// CHECK: %[[SIZE:.*]] = llvm.mlir.constant(4 : i64) : i64
// CHECK: %[[PTR:.*]] = llvm.call @brt_mmtk_alloc(%[[SIZE]]) : (i64) -> !llvm.ptr
// CHECK: %[[VAL:.*]] = llvm.mlir.constant(12 : i32) : i32
// CHECK: llvm.store %[[VAL]], %[[PTR]] : i32, !llvm.ptr
// CHECK: %[[LOAD:.*]] = llvm.load %[[PTR]] : !llvm.ptr -> i32
// CHECK: llvm.return %[[LOAD]] : i32

bir.func @main() -> !bir.int {
  %0 = bir.var.declare "x" : !bir.ref<!bir.int>
  %1 = bir.constant 12 : !bir.int
  bir.var.store %1 to %0 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.var.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %2 : !bir.int
}
