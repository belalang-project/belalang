// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK:      module {
// CHECK-NEXT:   llvm.func @brt_gc_init()
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     llvm.call @brt_gc_init() : () -> ()
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(0 : i64) : i64
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant #bir.int<0> : !bir.int
  bir.return
}

// -----

// CHECK:      module {
// CHECK-NEXT:   llvm.func @brt_gc_init()
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     llvm.call @brt_gc_init() : () -> ()
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(0.000000e+00 : f64) : f64
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant #bir.float<0.00> : !bir.float
  bir.return
}

// -----

// CHECK:      module {
// CHECK-NEXT:   llvm.func @brt_gc_init()
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     llvm.call @brt_gc_init() : () -> ()
// CHECK-NEXT:     %[[C0:.*]] = llvm.mlir.constant(1.230000e+00 : f64) : f64
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  %0 = bir.constant #bir.float<1.23> : !bir.float
  bir.return
}

// -----

// CHECK: llvm.func @brt_gc_init()
// CHECK: llvm.func @f(i64) -> i64
// CHECK: llvm.func @g(i64)

// CHECK-LABEL: llvm.func @main() -> i64
// CHECK: llvm.call @brt_gc_init() : () -> ()
// CHECK: %[[C1:.*]] = llvm.mlir.constant(1 : i64) : i64
// CHECK: %[[CALL:.*]] = llvm.call @f(%[[C1]]) : (i64) -> i64
// CHECK: llvm.call @f(%[[C1]]) : (i64) -> i64
// CHECK: llvm.return %[[CALL]] : i64

bir.func @f(%arg0 : !bir.int) -> !bir.int
bir.func @g(%arg0 : !bir.int)

bir.func @main() -> !bir.int {
  %0 = bir.constant #bir.int<1> : !bir.int
  %1 = bir.call @f(%0) : (!bir.int) -> !bir.int
  bir.call @f(%0) : (!bir.int) -> !bir.int
  bir.return %1 : !bir.int
}

// -----

// CHECK: llvm.func @brt_gc_init()
// CHECK: llvm.func @brt_gc_alloc(i64) -> !llvm.ptr

// CHECK-LABEL: llvm.func @main() -> i64
// CHECK: llvm.call @brt_gc_init() : () -> ()
// CHECK: %[[SIZE:.*]] = llvm.mlir.constant(8 : i64) : i64
// CHECK: %[[PTR:.*]] = llvm.call @brt_gc_alloc(%[[SIZE]]) : (i64) -> !llvm.ptr
// CHECK: %[[VAL:.*]] = llvm.mlir.constant(12 : i64) : i64
// CHECK: llvm.store %[[VAL]], %[[PTR]] : i64, !llvm.ptr
// CHECK: %[[LOAD:.*]] = llvm.load %[[PTR]] : !llvm.ptr -> i64
// CHECK: llvm.return %[[LOAD]] : i64

bir.func @main() -> !bir.int {
  %0 = bir.var.declare "x" : !bir.ref<!bir.int>
  %1 = bir.constant #bir.int<12> : !bir.int
  bir.var.store %1 to %0 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.var.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %2 : !bir.int
}

// -----

// CHECK:      llvm.func @brt_gc_init()
// CHECK-NEXT: llvm.func @f()
// CHECK-LABEL: llvm.func @main()
// CHECK:        llvm.call @brt_gc_init() : () -> ()
// CHECK-NEXT:   %[[ADDR:.*]] = llvm.mlir.addressof @f : !llvm.ptr
// CHECK-NEXT:   llvm.return
bir.func @f()

bir.func @main() {
  %0 = bir.constant #bir.fn<@f> : () -> ()
  bir.return
}
