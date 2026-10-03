// RUN: %bir-opt --verify-roundtrip --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @main
// CHECK-SAME:      (%[[ARG:.*]]: !gc.ptr<i64>)
// CHECK-NEXT:    %[[VALUE:.*]] = gc.load %[[ARG]] : !gc.ptr<i64>
// CHECK-NEXT:    return %[[VALUE]] : i64
func.func @main(%a : !gc.ptr<i64>) -> i64 {
  %value = gc.load %a : !gc.ptr<i64>
  return %value : i64
}

// -----

// CHECK-LABEL: func.func @loads_a_derived_pointer
// CHECK-SAME:      (%[[ARG:.*]]: !gc.derived_ptr<i64>)
// CHECK-NEXT:    %[[VALUE:.*]] = gc.load %[[ARG]] : !gc.derived_ptr<i64>
// CHECK-NEXT:    return %[[VALUE]] : i64
func.func @loads_a_derived_pointer(%a : !gc.derived_ptr<i64>) -> i64 {
  %value = gc.load %a : !gc.derived_ptr<i64>
  return %value : i64
}
