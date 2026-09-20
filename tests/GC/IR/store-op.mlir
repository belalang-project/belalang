// RUN: %bir-opt --verify-roundtrip --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @main
// CHECK-SAME:      (%[[VAL:.*]]: i64, %[[PTR:.*]]: !gc.ptr<i64>)
// CHECK-NEXT:    gc.store %[[VAL]], %[[PTR]] : !gc.ptr<i64>
// CHECK-NEXT:    return
func.func @main(%value : i64, %ptr : !gc.ptr<i64>) {
  gc.store %value, %ptr : !gc.ptr<i64>
  return
}
