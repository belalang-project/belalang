// RUN: %bir-opt --verify-roundtrip --split-input-file %s | %FileCheck %s

// CHECK-LABEL: func.func @main
// CHECK-NEXT:    %[[VALUE:.*]] = gc.alloca : !gc.ptr<i64>
// CHECK-NEXT:    return %[[VALUE]] : !gc.ptr<i64>
func.func @main() -> !gc.ptr<i64> {
  %value = gc.alloca : !gc.ptr<i64>
  return %value : !gc.ptr<i64>
}
