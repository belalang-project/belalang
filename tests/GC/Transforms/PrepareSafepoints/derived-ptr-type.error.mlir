// RUN: %not %bir-opt --allow-unregistered-dialect --split-input-file --gc-prepare-safepoints %s 2>&1 | %FileCheck %s

// CHECK: error: 'gc.alloc' op does not support derived pointers live across safepoints

func.func @derived_result(%base : !gc.ptr<i8>, %offset : index) {
  %derived = gc.ptr_offset %base, %offset
      : !gc.ptr<i8>, index -> !gc.derived_ptr<i8>
  %object = gc.alloc : !gc.ptr<i8>
  "test.use"(%derived) : (!gc.derived_ptr<i8>) -> ()
  return
}

// -----

// CHECK: error: 'gc.alloc' op does not support derived pointers live across safepoints

func.func @derived_argument(%derived : !gc.derived_ptr<i8>) {
  %object = gc.alloc : !gc.ptr<i8>
  "test.use"(%derived) : (!gc.derived_ptr<i8>) -> ()
  return
}
