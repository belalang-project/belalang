// RUN: %bir-opt --verify-roundtrip --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @main
// CHECK-SAME:      (%[[SOURCE:.*]]: !gc.ptr<i8>) -> !gc.ptr<i32>
// CHECK-NEXT:    %[[RESULT:.*]] = gc.ptr_cast %[[SOURCE]] : !gc.ptr<i8> -> !gc.ptr<i32>
// CHECK-NEXT:    return %[[RESULT]] : !gc.ptr<i32>

func.func @main(%a : !gc.ptr<i8>) -> !gc.ptr<i32> {
  %b = gc.ptr_cast %a : !gc.ptr<i8> -> !gc.ptr<i32>
  return %b : !gc.ptr<i32>
}
