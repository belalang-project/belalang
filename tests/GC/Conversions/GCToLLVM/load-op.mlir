// RUN: %bir-opt --convert-gcir-to-llvm --split-input-file %s | %FileCheck %s

// CHECK-LABEL: llvm.func @main
// CHECK-SAME:      (%[[ARG:.*]]: !llvm.ptr)
// CHECK-NEXT:    %[[VALUE:.*]] = llvm.load %[[ARG]] : !llvm.ptr -> i64
// CHECK-NEXT:    llvm.return %[[VALUE]] : i64
func.func @main(%a : !gc.ptr<i64>) -> i64 {
  %value = gc.load %a : !gc.ptr<i64>
  return %value : i64
}
