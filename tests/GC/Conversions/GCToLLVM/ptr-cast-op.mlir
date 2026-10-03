// RUN: %bir-opt --split-input-file --convert-gc-to-llvm %s | %FileCheck %s
// RUN: %bir-opt --split-input-file --convert-to-llvm %s | %FileCheck %s

// CHECK-LABEL: llvm.func @main
// CHECK-SAME:      (%[[SOURCE:.*]]: !llvm.ptr) -> !llvm.ptr
// CHECK-NEXT:    llvm.return %[[SOURCE]] : !llvm.ptr
func.func @main(%source : !gc.ptr<i8>) -> !gc.ptr<i32> {
  %result = gc.ptr_cast %source : !gc.ptr<i8> -> !gc.ptr<i32>
  return %result : !gc.ptr<i32>
}

// -----

// CHECK-LABEL: llvm.func @cast_derived
// CHECK-SAME:      (%[[SOURCE:.*]]: !llvm.ptr) -> !llvm.ptr
// CHECK-NEXT:    llvm.return %[[SOURCE]] : !llvm.ptr
func.func @cast_derived(%source : !gc.derived_ptr<i8>)
    -> !gc.derived_ptr<i32> {
  %result = gc.ptr_cast %source
      : !gc.derived_ptr<i8> -> !gc.derived_ptr<i32>
  return %result : !gc.derived_ptr<i32>
}
