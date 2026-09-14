// RUN: %bir-opt --gc-lower-allocations %s | %FileCheck %s
// RUN: %bir-opt --gc-lower-allocations='alloc-function=my_alloc' %s | %FileCheck %s --check-prefix=CUSTOM

// CHECK-LABEL: func.func @main
// CHECK:         %[[OBJ:.*]] = gc.call @malloc() : () -> !gc.ptr<i64>
// CHECK:         return %[[OBJ]] : !gc.ptr<i64>

// CUSTOM-LABEL: func.func @main
// CUSTOM:         %[[OBJ:.*]] = gc.call @my_alloc() : () -> !gc.ptr<i64>
// CUSTOM:         return %[[OBJ]] : !gc.ptr<i64>

func.func @main() -> !gc.ptr<i64> {
  %object = gc.alloc : !gc.ptr<i64>
  return %object : !gc.ptr<i64>
}
