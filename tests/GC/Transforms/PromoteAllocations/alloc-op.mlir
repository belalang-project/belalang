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
