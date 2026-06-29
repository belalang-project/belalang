// RUN: %bir-opt --split-input-file --bir-lowering-pipeline --convert-bir-to-llvm %s | %FileCheck %s

// CHECK:      module {
// CHECK-NEXT:   llvm.func @brt_mmtk_init()
// CHECK-NEXT:   llvm.func @main() -> i64 {
// CHECK-NEXT:     llvm.call @brt_mmtk_init() : () -> ()
// CHECK-NEXT:     llvm.br ^bb1
// CHECK-NEXT:   ^bb1:  // pred: ^bb0
// CHECK-NEXT:     %[[VAL:.*]] = llvm.mlir.constant(42 : i64) : i64
// CHECK-NEXT:     llvm.br ^bb2(%[[VAL]] : i64)
// CHECK-NEXT:   ^bb2(%[[ARG:.*]]: i64):  // pred: ^bb1
// CHECK-NEXT:     llvm.return %[[ARG]] : i64
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() -> !bir.int {
  %0 = bir.scope {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.yield %1 : !bir.int
  } : !bir.int
  bir.return %0 : !bir.int
}

// -----

// CHECK:      module {
// CHECK-NEXT:   llvm.func @brt_mmtk_init()
// CHECK-NEXT:   llvm.func @main() {
// CHECK-NEXT:     llvm.call @brt_mmtk_init() : () -> ()
// CHECK-NEXT:     llvm.br ^bb1
// CHECK-NEXT:   ^bb1:  // pred: ^bb0
// CHECK-NEXT:     %[[VAL:.*]] = llvm.mlir.constant(42 : i64) : i64
// CHECK-NEXT:     llvm.br ^bb2
// CHECK-NEXT:   ^bb2:  // pred: ^bb1
// CHECK-NEXT:     llvm.return
// CHECK-NEXT:   }
// CHECK-NEXT: }

bir.func @main() {
  bir.scope {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.yield
  }
  bir.return
}
