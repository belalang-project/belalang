// RUN: %bir-opt --split-input-file --bir-flatten-cfg --mem2reg %s | %FileCheck %s

// CHECK-LABEL: bir.func @if_join
// CHECK-NOT:   bir.alloc_stack
// CHECK:       bir.cond_br %{{.*}} ^bb1, ^bb2
// CHECK:     ^bb1:
// CHECK:       %[[ONE:.*]] = bir.constant #bir.int<1> : !bir.int
// CHECK:       cf.br ^bb3(%[[ONE]] : !bir.int)
// CHECK:     ^bb2:
// CHECK:       %[[TWO:.*]] = bir.constant #bir.int<2> : !bir.int
// CHECK:       cf.br ^bb3(%[[TWO]] : !bir.int)
// CHECK:     ^bb3(%[[JOIN:.*]]: !bir.int):
// CHECK:       call @use(%[[JOIN]]) : (!bir.int) -> ()
// CHECK-NEXT:  bir.return
bir.func @use(!bir.int)

bir.func @if_join() {
  %0 = bir.alloc_stack : !bir.ref<!bir.int>
  %1 = bir.constant #bir.bool<true> : !bir.bool
  bir.if %1 {
    %2 = bir.constant #bir.int<1> : !bir.int
    bir.store %2 to %0 : !bir.int to !bir.ref<!bir.int>
    bir.yield
  } else {
    %3 = bir.constant #bir.int<2> : !bir.int
    bir.store %3 to %0 : !bir.int to !bir.ref<!bir.int>
    bir.yield
  }
  %4 = bir.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.call @use(%4) : (!bir.int) -> ()
  bir.return
}

// -----

// CHECK-LABEL: bir.func @loop_carried
// CHECK-NOT:   bir.alloc_stack
// CHECK:       %[[ZERO:.*]] = bir.constant #bir.int<0> : !bir.int
// CHECK:       cf.br ^bb1(%[[ZERO]] : !bir.int)
// CHECK:     ^bb1(%[[ITER:.*]]: !bir.int):
// CHECK:       bir.cond_br %{{.*}} ^bb2, ^bb3
// CHECK:     ^bb2:
// CHECK:       %[[ONE:.*]] = bir.constant #bir.int<1> : !bir.int
// CHECK:       cf.br ^bb1(%[[ONE]] : !bir.int)
// CHECK:     ^bb3:
// CHECK:       call @use(%[[ITER]]) : (!bir.int) -> ()
// CHECK-NEXT:  bir.return
bir.func @use(!bir.int)

bir.func @loop_carried() {
  %0 = bir.alloc_stack : !bir.ref<!bir.int>
  %1 = bir.constant #bir.int<0> : !bir.int
  bir.store %1 to %0 : !bir.int to !bir.ref<!bir.int>
  bir.while {
    %2 = bir.constant #bir.bool<true> : !bir.bool
    bir.condition %2
  } do {
    %3 = bir.constant #bir.int<1> : !bir.int
    bir.store %3 to %0 : !bir.int to !bir.ref<!bir.int>
    bir.yield
  }
  %4 = bir.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.call @use(%4) : (!bir.int) -> ()
  bir.return
}
