// RUN: %bir-opt --allow-unregistered-dialect --split-input-file --gcir-prepare-gc-safepoints %s | %FileCheck %s

// CHECK-LABEL: func.func @live_root
// CHECK:         %[[OBJ:.*]], %[[ROOT_NEXT:.*]] = gc.alloc roots(%arg0 : !gc.ptr<i64>) : !gc.ptr<i64>
// CHECK:         "test.use"(%[[ROOT_NEXT]]) : (!gc.ptr<i64>) -> ()
func.func @live_root(%root : !gc.ptr<i64>) {
  %object = gc.alloc : !gc.ptr<i64>
  "test.use"(%root) : (!gc.ptr<i64>) -> ()
  "test.use"(%object) : (!gc.ptr<i64>) -> ()
  return
}

// -----

// CHECK-LABEL: func.func @dead_root
// CHECK:         %[[OBJ:.*]] = gc.alloc : !gc.ptr<i64>
func.func @dead_root(%root : !gc.ptr<i64>) {
  "test.use"(%root) : (!gc.ptr<i64>) -> ()
  %object = gc.alloc : !gc.ptr<i64>
  "test.use"(%object) : (!gc.ptr<i64>) -> ()
  return
}
