// RUN: %not %bir-opt --verify-roundtrip --split-input-file %s 2>&1 | %FileCheck %s

// CHECK: error: 'gc.ptr_offset' op result #0 must be derived_ptr, but got '!gc.ptr<i8>'

func.func @main(%a : !gc.ptr<i32>) {
  %off24 = arith.constant 24 : index
  %b = gc.ptr_offset %a, %off24 : !gc.ptr<i32>, index -> !gc.ptr<i8>
  return
}
