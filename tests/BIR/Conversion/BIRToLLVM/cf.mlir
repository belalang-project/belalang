// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK-LABEL:  llvm.func @main() -> i64 {
// CHECK-NEXT:     llvm.br ^bb1
// CHECK-NEXT:   ^bb1:  // pred: ^bb0
// CHECK-NEXT:     %[[VAL:.*]] = llvm.mlir.constant(42 : i64) : i64
// CHECK-NEXT:     llvm.br ^bb2(%[[VAL]] : i64)
// CHECK-NEXT:   ^bb2(%[[ARG:.*]]: i64):  // pred: ^bb1
// CHECK-NEXT:     llvm.return %[[ARG]] : i64
// CHECK-NEXT:   }

bir.func @main() -> !bir.int {
  cf.br ^bb1
^bb1:
  %0 = bir.constant #bir.int<42> : !bir.int
  cf.br ^bb2(%0 : !bir.int)
^bb2(%1: !bir.int):
  bir.return %1 : !bir.int
}

// -----

// CHECK-LABEL:  llvm.func @main() {
// CHECK-NEXT:     llvm.br ^bb1
// CHECK-NEXT:   ^bb1:  // pred: ^bb0
// CHECK-NEXT:     %0 = llvm.mlir.constant(42 : i64) : i64
// CHECK-NEXT:     llvm.br ^bb2
// CHECK-NEXT:   ^bb2:  // pred: ^bb1
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }

bir.func @main() {
  cf.br ^bb1
^bb1:
  %0 = bir.constant #bir.int<42> : !bir.int
  cf.br ^bb2
^bb2:
  bir.return
}
