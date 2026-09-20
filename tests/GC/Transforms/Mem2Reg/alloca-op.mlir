// RUN: %bir-opt --mem2reg --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @store_then_load
// CHECK-SAME:      (%[[VALUE:.*]]: i64)
// CHECK-NEXT:    return %[[VALUE]] : i64

func.func @store_then_load(%value : i64) -> i64 {
  %slot = gc.alloca : !gc.ptr<i64>
  gc.store %value, %slot : !gc.ptr<i64>
  %loaded = gc.load %slot : !gc.ptr<i64>
  return %loaded : i64
}
