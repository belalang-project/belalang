// RUN: %bir-opt --verify-roundtrip --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @main
// CHECK-SAME:      (%[[ARG0:.*]]: !gc.ptr<i32>)
// CHECK-NEXT:    %[[C24:.*]] = arith.constant 24 : index
// CHECK-NEXT:    %[[RESULT:.*]] = gc.ptr_offset %[[ARG0]], %[[C24]] : !gc.ptr<i32>, index -> !gc.ptr<i8>
// CHECK-NEXT:    return

func.func @main(%a : !gc.ptr<i32>) {
  %off24 = arith.constant 24 : index
  %b = gc.ptr_offset %a, %off24 : !gc.ptr<i32>, index -> !gc.ptr<i8>
  return
}
