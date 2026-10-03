// RUN: %bir-opt --split-input-file --convert-gc-to-llvm %s | %FileCheck %s
// RUN: %bir-opt --split-input-file --convert-to-llvm %s | %FileCheck %s

// CHECK-LABEL: llvm.func @main
// CHECK-SAME:      (%[[VAL:.*]]: i64, %[[PTR:.*]]: !llvm.ptr)
// CHECK-NEXT:    llvm.store %[[VAL]], %[[PTR]] : i64, !llvm.ptr
// CHECK-NEXT:    llvm.return
func.func @main(%value : i64, %ptr : !gc.ptr<i64>) {
  gc.store %value, %ptr : !gc.ptr<i64>
  return
}

// -----

// CHECK-LABEL: llvm.func @stores_to_derived_ptr
// CHECK-SAME:      (%[[VAL:.*]]: i64, %[[PTR:.*]]: !llvm.ptr)
// CHECK-NEXT:    llvm.store %[[VAL]], %[[PTR]] : i64, !llvm.ptr
// CHECK-NEXT:    llvm.return
func.func @stores_to_derived_ptr(%value : i64, %ptr : !gc.derived_ptr<i64>) {
  gc.store %value, %ptr : !gc.derived_ptr<i64>
  return
}
