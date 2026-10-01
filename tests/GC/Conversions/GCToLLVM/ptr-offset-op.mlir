// RUN: %bir-opt --split-input-file --convert-to-llvm %s | %FileCheck %s

// CHECK-LABEL: llvm.func @main
// CHECK-SAME:      (%[[ARG:.*]]: !llvm.ptr)
// CHECK-NEXT:    %[[OFFSET:.*]] = llvm.mlir.constant(24 : i64) : i64
// CHECK-NEXT:    %[[RESULT:.*]] = llvm.getelementptr %arg0[%[[OFFSET]]] : (!llvm.ptr, i64) -> !llvm.ptr, i8
// CHECK-NEXT:    llvm.return

func.func @main(%a : !gc.ptr<i8>) {
  %off24 = arith.constant 24 : index
  %b = gc.ptr_offset %a, %off24 : !gc.ptr<i8>, index -> !gc.ptr<i8>
  return
}
