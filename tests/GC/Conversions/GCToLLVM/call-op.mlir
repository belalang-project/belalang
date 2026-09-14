// RUN: %bir-opt --verify-roundtrip --convert-gcir-to-llvm %s | %FileCheck %s

// CHECK-LABEL: llvm.func @main
// CHECK-NEXT:    %[[RESULT:.*]] = llvm.call @callee() : () -> !llvm.ptr
// CHECK-NEXT:    llvm.return %[[RESULT]] : !llvm.ptr
func.func private @callee() -> !gc.ptr<i64>

func.func @main() -> !gc.ptr<i64> {
  %result = gc.call @callee() : () -> !gc.ptr<i64>
  return %result : !gc.ptr<i64>
}
