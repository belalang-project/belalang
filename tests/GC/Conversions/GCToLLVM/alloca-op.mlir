// RUN: %bir-opt --verify-roundtrip --split-input-file --convert-gcir-to-llvm %s | %FileCheck %s

// CHECK-LABEL: llvm.func @main
// CHECK-NEXT:    %[[SZ:.*]] = llvm.mlir.constant(1 : i64) : i64
// CHECK-NEXT:    %[[PTR:.*]] = llvm.alloca %[[SZ]] x i64 : (i64) -> !llvm.ptr
// CHECK-NEXT:    llvm.return %[[PTR]] : !llvm.ptr
func.func @main() -> !gc.ptr<i64> {
  %value = gc.alloca : !gc.ptr<i64>
  return %value : !gc.ptr<i64>
}
