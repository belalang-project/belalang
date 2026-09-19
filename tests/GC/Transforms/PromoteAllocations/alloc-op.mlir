// RUN: %bir-opt --allow-unregistered-dialect --split-input-file --gc-promote-allocations %s \
// RUN: | %FileCheck %s

// CHECK-LABEL: func.func @promote_non_escaping
// CHECK-NEXT:    %[[OBJECT:.*]] = gc.alloca : !gc.ptr<i64>
// CHECK-NEXT:    "test.use"(%[[OBJECT]])
// CHECK-NEXT:    return

func.func @promote_non_escaping() {
  %object = gc.alloc : !gc.ptr<i64>
  "test.use"(%object) : (!gc.ptr<i64>) -> ()
  return
}

// -----

// CHECK-LABEL: func.func @keep_escaping
// CHECK-NEXT:    %[[OBJECT:.*]] = gc.alloc : !gc.ptr<i64>
// CHECK-NEXT:    return %[[OBJECT]] : !gc.ptr<i64>

func.func @keep_escaping() -> !gc.ptr<i64> {
  %object = gc.alloc : !gc.ptr<i64>
  return %object : !gc.ptr<i64>
}

// -----

// CHECK-LABEL: func.func @promote_with_roots
// CHECK-NEXT:    %[[ROOT:.*]] = gc.alloca : !gc.ptr<i64>
// CHECK-NEXT:    %[[OBJECT:.*]] = gc.alloca : !gc.ptr<i64>
// CHECK-NEXT:    "test.use"(%[[OBJECT]])
// CHECK-NEXT:    "test.use"(%[[ROOT]])
// CHECK-NEXT:    return

func.func @promote_with_roots() {
  %root.0 = gc.alloc : !gc.ptr<i64>

  %obj.0, %root.1 = gc.alloc
      roots(%root.0 : !gc.ptr<i64>) : !gc.ptr<i64>

  "test.use"(%obj.0) : (!gc.ptr<i64>) -> ()
  "test.use"(%root.1) : (!gc.ptr<i64>) -> ()
  return
}
