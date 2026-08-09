// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK-LABEL: llvm.func @alloc_stack_int
bir.func @alloc_stack_int() {
  // CHECK:      %[[C0:.*]] = llvm.mlir.constant(1 : i64) : i64
  // CHECK-NEXT: %[[C1:.*]] = llvm.alloca %[[C0]] x i64 : (i64) -> !llvm.ptr
  %1 = bir.alloc_stack : !bir.ref<!bir.int>
  bir.return
}

// -----

// CHECK-LABEL: llvm.func @alloc_stack_struct
bir.func @alloc_stack_struct() {
  // CHECK:      %[[C0:.*]] = llvm.mlir.constant(1 : i64) : i64
  // CHECK-NEXT: %[[C1:.*]] = llvm.alloca %[[C0]] x !llvm.struct<"T", (i64, i64)> : (i64) -> !llvm.ptr
  %1 = bir.alloc_stack : !bir.ref<!bir.struct<"T", {!bir.int, !bir.int}>>
  bir.return
}
