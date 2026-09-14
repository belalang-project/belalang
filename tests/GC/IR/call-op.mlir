// RUN: %bir-opt %s | %FileCheck %s

// CHECK:      func.func @main(%[[ARG:.*]]: i64) -> !gc.ptr<i64> {
// CHECK-NEXT:   %[[RESULT:.*]] = gc.call @allocate(%[[ARG]]) : (i64) -> !gc.ptr<i64>
// CHECK-NEXT:   return %[[RESULT]] : !gc.ptr<i64>
// CHECK-NEXT: }
func.func @main(%arg : i64) -> !gc.ptr<i64> {
  %result = gc.call @allocate(%arg) : (i64) -> !gc.ptr<i64>
  return %result : !gc.ptr<i64>
}
