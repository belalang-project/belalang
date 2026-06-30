// RUN: %bir-opt --split-input-file --bir-flatten-cfg %s | %FileCheck %s

// CHECK:        cf.br ^bb1
// CHECK-NEXT: ^bb1:  // 2 preds: ^bb0, ^bb2
// CHECK-NEXT:   %0 = bir.constant #bir.bool<true> : !bir.bool
// CHECK-NEXT:   bir.cond_br %0 ^bb2, ^bb3
// CHECK-NEXT: ^bb2:  // pred: ^bb1
// CHECK-NEXT:   %1 = bir.constant #bir.int<1> : !bir.int
// CHECK-NEXT:   bir.print %1 : !bir.int
// CHECK-NEXT:   cf.br ^bb1
// CHECK-NEXT: ^bb3:  // pred: ^bb1
// CHECK-NEXT:   bir.return

bir.func @main() {
  bir.while {
    %cond = bir.constant #bir.bool<true> : !bir.bool
    bir.condition %cond
  } do {
    %v = bir.constant #bir.int<1> : !bir.int
    bir.print %v : !bir.int
    bir.continue
  }
  bir.return
}

// -----

// CHECK:        cf.br ^bb1
// CHECK-NEXT: ^bb1:  // 2 preds: ^bb0, ^bb4
// CHECK-NEXT:   %0 = bir.constant #bir.bool<true> : !bir.bool
// CHECK-NEXT:   bir.cond_br %0 ^bb2, ^bb5
// CHECK-NEXT: ^bb2:  // pred: ^bb1
// CHECK-NEXT:   %1 = bir.constant #bir.bool<true> : !bir.bool
// CHECK-NEXT:   bir.cond_br %1 ^bb3, ^bb4
// CHECK-NEXT: ^bb3:  // pred: ^bb2
// CHECK-NEXT:   cf.br ^bb5
// CHECK-NEXT: ^bb4:  // pred: ^bb2
// CHECK-NEXT:   cf.br ^bb1
// CHECK-NEXT: ^bb5:  // 2 preds: ^bb1, ^bb3
// CHECK-NEXT:   bir.return

bir.func @main() {
  bir.while {
    %cond = bir.constant #bir.bool<true> : !bir.bool
    bir.condition %cond
  } do {
    %cond = bir.constant #bir.bool<true> : !bir.bool
    bir.if %cond {
      bir.break
    }
    bir.continue
  }
  bir.return
}
