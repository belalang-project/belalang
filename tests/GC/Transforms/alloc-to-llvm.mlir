// RUN: %bir-opt --convert-gcir-to-llvm %s | %FileCheck %s
// RUN: %bir-opt --convert-gcir-to-llvm='alloc-function=my_alloc' %s | %FileCheck %s --check-prefix=CUSTOM

// CHECK-LABEL: llvm.func @malloc
// CHECK-SAME: (i64) -> !llvm.ptr

// CHECK-LABEL: llvm.func @test
// CHECK: %[[SIZE:.*]] = llvm.mlir.constant(8 : i64) : i64
// CHECK: %[[PTR:.*]] = llvm.call @malloc(%[[SIZE]]) : (i64) -> !llvm.ptr
// CHECK: llvm.return %[[PTR]] : !llvm.ptr

// CUSTOM-LABEL: llvm.func @my_alloc
// CUSTOM-SAME: (i64) -> !llvm.ptr

// CUSTOM: llvm.call @my_alloc

module {
  func.func @test() -> !gc.ptr<i64> {
    %p = "gc.alloc"() : () -> !gc.ptr<i64>
    return %p : !gc.ptr<i64>
  }
}
