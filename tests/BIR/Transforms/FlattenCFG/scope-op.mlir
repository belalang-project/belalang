// RUN: %bir-opt --split-input-file --bir-flatten-cfg %s | %FileCheck %s

// CHECK:        cf.br ^bb1
// CHECK-NEXT: ^bb1:  // pred: ^bb0
// CHECK-NEXT:   %0 = bir.constant #bir.int<42> : !bir.int
// CHECK-NEXT:   cf.br ^bb2(%0 : !bir.int)
// CHECK-NEXT: ^bb2(%1: !bir.int):  // pred: ^bb1
// CHECK-NEXT:   bir.return %1 : !bir.int

bir.func @main() -> !bir.int {
  %0 = bir.scope {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.yield %1 : !bir.int
  } : !bir.int
  bir.return %0 : !bir.int
}

// -----

// CHECK:        cf.br ^bb1
// CHECK-NEXT: ^bb1:  // pred: ^bb0
// CHECK-NEXT:   %0 = bir.constant #bir.int<42> : !bir.int
// CHECK-NEXT:   cf.br ^bb2
// CHECK-NEXT: ^bb2:  // pred: ^bb1
// CHECK-NEXT:   bir.return

bir.func @main() {
  bir.scope {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.yield
  }
  bir.return
}
