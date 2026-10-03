// RUN: %not %bir-opt --verify-roundtrip --split-input-file %s 2>&1 | %FileCheck %s

// CHECK: error: 'gc.ptr_cast' op must preserve whether the pointer is derived

func.func @main(%a : !gc.ptr<i8>) {
  %b = gc.ptr_cast %a : !gc.ptr<i8> -> !gc.derived_ptr<i32>
  return
}

// -----

// CHECK: error: 'gc.ptr_cast' op must preserve whether the pointer is derived

func.func @main(%a : !gc.derived_ptr<i8>) {
  %b = gc.ptr_cast %a : !gc.derived_ptr<i8> -> !gc.ptr<i32>
  return
}
