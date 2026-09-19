// RUN: %bir-opt --allow-unregistered-dialect --split-input-file --gc-promote-allocations %s \
// RUN: | %FileCheck %s

// CHECK-LABEL: func.func @main
func.func @main() -> !gc.ptr<i64> {
  // CHECK-NOT: gc.alloc : !gc.ptr<i64>
  // CHECK:     gc.alloca : !gc.ptr<i64>
  %a = gc.alloc : !gc.ptr<i64>
  "test.use"(%a) : (!gc.ptr<i64>) -> ()

  // CHECK: gc.alloc : !gc.ptr<i64>
  %b = gc.alloc : !gc.ptr<i64>
  return %b : !gc.ptr<i64>
}

// -----

// CHECK-LABEL: func.func @main
func.func @main() {
  // CHECK-NOT: gc.alloc : !gc.ptr<i64>
  // CHECK:     gc.alloca : !gc.ptr<i64>
  %a.0 = gc.alloc : !gc.ptr<i64>
  "test.use"(%a.0) : (!gc.ptr<i64>) -> ()

  // CHECK-NOT: gc.alloc : !gc.ptr<i64>
  // CHECK:     gc.alloca : !gc.ptr<i64>
  %b.0, %a.1 = gc.alloc roots(%a.0 : !gc.ptr<i64>) : !gc.ptr<i64>
  "test.use"(%b.0) : (!gc.ptr<i64>) -> ()
  
  // CHECK: return
  return
}
