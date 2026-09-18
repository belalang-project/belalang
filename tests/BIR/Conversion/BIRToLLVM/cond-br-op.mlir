// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK:        %0 = llvm.mlir.constant(true) : i1
// CHECK-NEXT:   llvm.cond_br %0, ^bb1, ^bb2
// CHECK-NEXT: ^bb1:  // pred: ^bb0
// CHECK-NEXT:   llvm.br ^bb3
// CHECK-NEXT: ^bb2:  // pred: ^bb0
// CHECK-NEXT:   llvm.br ^bb3
// CHECK-NEXT: ^bb3:  // 2 preds: ^bb1, ^bb2
// CHECK-NEXT:   llvm.return

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  bir.cond_br %0 ^bb1, ^bb2
^bb1:
  cf.br ^bb3
^bb2:
  cf.br ^bb3
^bb3:
  bir.return
}
