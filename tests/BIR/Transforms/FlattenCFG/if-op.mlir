// RUN: %bir-opt --split-input-file --bir-flatten-cfg %s | %FileCheck %s

// CHECK:        %0 = bir.constant #bir.bool<true> : !bir.bool
// CHECK-NEXT:   bir.cond_br %0 ^bb1, ^bb2
// CHECK-NEXT: ^bb1:  // pred: ^bb0
// CHECK-NEXT:   %1 = bir.constant #bir.int<1> : !bir.int
// CHECK-NEXT:   bir.print %1 : !bir.int
// CHECK-NEXT:   cf.br ^bb3
// CHECK-NEXT: ^bb2:  // pred: ^bb0
// CHECK-NEXT:   %2 = bir.constant #bir.int<2> : !bir.int
// CHECK-NEXT:   bir.print %2 : !bir.int
// CHECK-NEXT:   cf.br ^bb3
// CHECK-NEXT: ^bb3:  // 2 preds: ^bb1, ^bb2
// CHECK-NEXT:   bir.return

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  bir.if %0 {
    %1 = bir.constant #bir.int<1> : !bir.int
    bir.print %1 : !bir.int
    bir.yield
  } else {
    %2 = bir.constant #bir.int<2> : !bir.int
    bir.print %2 : !bir.int
    bir.yield
  }
  bir.return
}

// -----

// CHECK:        %0 = bir.constant #bir.bool<true> : !bir.bool
// CHECK-NEXT:   bir.cond_br %0 ^bb1, ^bb2
// CHECK-NEXT: ^bb1:  // pred: ^bb0
// CHECK-NEXT:   %1 = bir.constant #bir.int<1> : !bir.int
// CHECK-NEXT:   bir.print %1 : !bir.int
// CHECK-NEXT:   cf.br ^bb2
// CHECK-NEXT: ^bb2:  // 2 preds: ^bb0, ^bb1
// CHECK-NEXT:   bir.return

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  bir.if %0 {
    %1 = bir.constant #bir.int<1> : !bir.int
    bir.print %1 : !bir.int
    bir.yield
  }
  bir.return
}

// -----

// CHECK:        %0 = bir.constant #bir.bool<true> : !bir.bool
// CHECK-NEXT:   bir.cond_br %0 ^bb1, ^bb2
// CHECK-NEXT: ^bb1:  // pred: ^bb0
// CHECK-NEXT:   %1 = bir.constant #bir.int<1> : !bir.int
// CHECK-NEXT:   cf.br ^bb3(%1 : !bir.int)
// CHECK-NEXT: ^bb2:  // pred: ^bb0
// CHECK-NEXT:   %2 = bir.constant #bir.int<2> : !bir.int
// CHECK-NEXT:   cf.br ^bb3(%2 : !bir.int)
// CHECK-NEXT: ^bb3(%3: !bir.int):  // 2 preds: ^bb1, ^bb2
// CHECK-NEXT:   bir.return

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  %3 = bir.if %0 {
    %1 = bir.constant #bir.int<1> : !bir.int
    bir.yield %1 : !bir.int
  } else {
    %2 = bir.constant #bir.int<2> : !bir.int
    bir.yield %2 : !bir.int
  } : !bir.int
  bir.return
}
