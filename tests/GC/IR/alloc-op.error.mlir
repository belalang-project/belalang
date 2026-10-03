// RUN: %not %bir-opt --verify-roundtrip --split-input-file %s 2>&1 | %FileCheck %s

// CHECK: error: custom op 'gc.alloc' 'roots' must be variadic of ptr, but got '!gc.derived_ptr<i64>'

func.func @rejects_derived_ptr_in_roots(%arg0 : !gc.derived_ptr<i64>) {
  %v2, %arg0.1 = gc.alloc roots(%arg0 : !gc.derived_ptr<i64>) : !gc.ptr<i64>
  return
}
