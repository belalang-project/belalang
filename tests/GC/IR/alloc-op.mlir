// RUN: %bir-opt --verify-roundtrip --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @main
// CHECK-NEXT:    %[[VALUE:.*]] = gc.alloc : !gc.ptr<i64>
// CHECK-NEXT:    return %[[VALUE]] : !gc.ptr<i64>
func.func @main() -> !gc.ptr<i64> {
  %value = gc.alloc : !gc.ptr<i64>
  return %value : !gc.ptr<i64>
}

// -----

// CHECK-LABEL: func.func @main
// CHECK-NEXT:    %[[V1:.*]] = gc.alloc : !gc.ptr<i64>
// CHECK-NEXT:    %[[V2:.*]], %[[V1_NEXT:.*]] = gc.alloc
// CHECK-SAME:        roots(%[[V1]] : !gc.ptr<i64>)
// CHECK-SAME:        : !gc.ptr<i64>
// CHECK-NEXT:    return %[[V1_NEXT]] : !gc.ptr<i64>
func.func @main() -> !gc.ptr<i64> {
  %v1 = gc.alloc : !gc.ptr<i64>
  %v2, %v1_next = gc.alloc roots(%v1 : !gc.ptr<i64>) : !gc.ptr<i64>
  return %v1_next : !gc.ptr<i64>
}

// -----

// CHECK-LABEL: func.func @multiple_roots
// CHECK:         %[[I64:.*]] = gc.alloc : !gc.ptr<i64>
// CHECK:         %[[F64:.*]] = gc.alloc : !gc.ptr<f64>
// CHECK:         %[[OBJECT:.*]], %[[RELOCATED:.*]]:2 = gc.alloc
// CHECK-SAME:      roots(%[[I64]], %[[F64]] : !gc.ptr<i64>, !gc.ptr<f64>)
// CHECK-SAME:      : !gc.ptr<i1>
// CHECK:         return %[[RELOCATED]]#0, %[[RELOCATED]]#1 : !gc.ptr<i64>, !gc.ptr<f64>
func.func @multiple_roots() -> (!gc.ptr<i64>, !gc.ptr<f64>) {
  %i64 = gc.alloc : !gc.ptr<i64>
  %f64 = gc.alloc : !gc.ptr<f64>
  %object, %i64_next, %f64_next = gc.alloc
      roots(%i64, %f64 : !gc.ptr<i64>, !gc.ptr<f64>) : !gc.ptr<i1>
  return %i64_next, %f64_next : !gc.ptr<i64>, !gc.ptr<f64>
}
