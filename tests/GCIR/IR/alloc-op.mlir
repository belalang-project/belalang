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
