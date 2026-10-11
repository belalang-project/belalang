// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK:      func.func @main(%[[ARG:.*]]: i64) -> !gc.ptr<i64> {
// CHECK-NEXT:   %[[RESULT:.*]] = gc.call @allocate(%[[ARG]]) : (i64) -> !gc.ptr<i64>
// CHECK-NEXT:   return %[[RESULT]] : !gc.ptr<i64>
// CHECK-NEXT: }
func.func @main(%arg : i64) -> !gc.ptr<i64> {
  %result = gc.call @allocate(%arg) : (i64) -> !gc.ptr<i64>
  return %result : !gc.ptr<i64>
}

// -----

// CHECK-LABEL: func.func @call_attributes
// CHECK-NEXT:    %[[RESULT:.*]] = gc.call @allocate(%{{.*}}) <arg_attrs = [{test.arg = "input"}], res_attrs = [{test.res = "output"}]> {test.marker = "preserved"} : (i64) -> !gc.ptr<i64>
// CHECK-NEXT:    return %[[RESULT]] : !gc.ptr<i64>
func.func @call_attributes(%arg : i64) -> !gc.ptr<i64> {
  %result = gc.call @allocate(%arg)
      <arg_attrs = [{test.arg = "input"}], res_attrs = [{test.res = "output"}]>
      {test.marker = "preserved"} : (i64) -> !gc.ptr<i64>
  return %result : !gc.ptr<i64>
}
