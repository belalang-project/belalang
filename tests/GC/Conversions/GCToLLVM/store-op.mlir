// RUN: %bir-opt --convert-gcir-to-llvm --split-input-file %s | %FileCheck %s

// CHECK-LABEL: llvm.func @main
// CHECK-SAME:      (%[[VAL:.*]]: i64, %[[PTR:.*]]: !llvm.ptr)
// CHECK-NEXT:    llvm.store %[[VAL]], %[[PTR]] : i64, !llvm.ptr
// CHECK-NEXT:    llvm.return
func.func @main(%value : i64, %ptr : !gc.ptr<i64>) {
  gc.store %value, %ptr : !gc.ptr<i64>
  return
}
