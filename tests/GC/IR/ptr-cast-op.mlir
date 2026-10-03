// RUN: %bir-opt --verify-roundtrip --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @main
// CHECK-SAME:      (%[[SOURCE:.*]]: !gc.ptr<i8>) -> !gc.ptr<i32>
// CHECK-NEXT:    %[[RESULT:.*]] = gc.ptr_cast %[[SOURCE]] : !gc.ptr<i8> -> !gc.ptr<i32>
// CHECK-NEXT:    return %[[RESULT]] : !gc.ptr<i32>

func.func @main(%a : !gc.ptr<i8>) -> !gc.ptr<i32> {
  %b = gc.ptr_cast %a : !gc.ptr<i8> -> !gc.ptr<i32>
  return %b : !gc.ptr<i32>
}

// -----

// CHECK-LABEL: func.func @cast_derived
// CHECK-SAME:      (%[[SOURCE:.*]]: !gc.derived_ptr<i8>) -> !gc.derived_ptr<i32>
// CHECK-NEXT:    %[[RESULT:.*]] = gc.ptr_cast %[[SOURCE]] : !gc.derived_ptr<i8> -> !gc.derived_ptr<i32>
// CHECK-NEXT:    return %[[RESULT]] : !gc.derived_ptr<i32>

func.func @cast_derived(%source : !gc.derived_ptr<i8>) -> !gc.derived_ptr<i32> {
  %result = gc.ptr_cast %source
      : !gc.derived_ptr<i8> -> !gc.derived_ptr<i32>
  return %result : !gc.derived_ptr<i32>
}
