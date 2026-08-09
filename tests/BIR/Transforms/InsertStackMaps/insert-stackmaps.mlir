// RUN: %bir-opt --split-input-file --bir-insert-stackmaps %s | %FileCheck %s

// CHECK-LABEL: bir.func @alloc_straight_line
// CHECK:        %0 = bir.constant #bir.int<1> : !bir.int
// CHECK-NEXT:   bir.safepoint 0
// CHECK-NEXT:   %1 = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK-NEXT:   bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
// CHECK-NEXT:   %2 = bir.constant #bir.float<1.000000e+00> : !bir.float
// CHECK-NEXT:   bir.safepoint 1(%1 : !bir.ref<!bir.int>)
// CHECK-NEXT:   %3 = bir.alloc_heap : !bir.ref<!bir.float>
// CHECK-NEXT:   bir.store %2 to %3 : !bir.float to !bir.ref<!bir.float>
// CHECK-NEXT:   %4 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
// CHECK-NEXT:   %5 = bir.constant #bir.int<1> : !bir.int
// CHECK-NEXT:   %6 = bir.add %4, %5 : (!bir.int, !bir.int) -> !bir.int
// CHECK-NEXT:   bir.safepoint 2(%1, %3 : !bir.ref<!bir.int>, !bir.ref<!bir.float>)
// CHECK-NEXT:   %7 = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK-NEXT:   bir.store %6 to %7 : !bir.int to !bir.ref<!bir.int>
// CHECK-NEXT:   %8 = bir.load %7 : (!bir.ref<!bir.int>) -> !bir.int
// CHECK-NEXT:   bir.return %8 : !bir.int
// CHECK-NEXT: }

bir.func @alloc_straight_line() -> !bir.int {
  %0 = bir.constant #bir.int<1> : !bir.int
  %1 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.constant #bir.float<1.000000e+00> : !bir.float
  %3 = bir.alloc_heap : !bir.ref<!bir.float>
  bir.store %2 to %3 : !bir.float to !bir.ref<!bir.float>
  %4 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
  %5 = bir.constant #bir.int<1> : !bir.int
  %6 = bir.add %4, %5 : (!bir.int, !bir.int) -> !bir.int
  %7 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.store %6 to %7 : !bir.int to !bir.ref<!bir.int>
  %8 = bir.load %7 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %8 : !bir.int
}

// -----

// CHECK-LABEL: bir.func @alloc_cross_block
// CHECK:        %0 = bir.constant #bir.int<0> : !bir.int
// CHECK-NEXT:   bir.safepoint 0
// CHECK-NEXT:   %1 = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK-NEXT:   bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
// CHECK-NEXT:   cf.br ^bb1
// CHECK-NEXT: ^bb1:  // pred: ^bb0
// CHECK-NEXT:   %2 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
// CHECK-NEXT:   %3 = bir.constant #bir.int<1> : !bir.int
// CHECK-NEXT:   %4 = bir.add %2, %3 : (!bir.int, !bir.int) -> !bir.int
// CHECK-NEXT:   bir.safepoint 1(%1 : !bir.ref<!bir.int>)
// CHECK-NEXT:   %5 = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK-NEXT:   bir.store %4 to %5 : !bir.int to !bir.ref<!bir.int>
// CHECK-NEXT:   %6 = bir.load %5 : (!bir.ref<!bir.int>) -> !bir.int
// CHECK-NEXT:   bir.return %6 : !bir.int
// CHECK-NEXT: }

bir.func @alloc_cross_block() -> !bir.int {
  %0 = bir.constant #bir.int<0> : !bir.int
  %1 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
  cf.br ^bb1
^bb1:
  %2 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
  %3 = bir.constant #bir.int<1> : !bir.int
  %4 = bir.add %2, %3 : (!bir.int, !bir.int) -> !bir.int
  %5 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.store %4 to %5 : !bir.int to !bir.ref<!bir.int>
  %6 = bir.load %5 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %6 : !bir.int
}

// -----

// CHECK-LABEL: bir.func @no_alloc
// CHECK:        %0 = bir.constant #bir.int<42> : !bir.int
// CHECK-NEXT:   bir.return %0 : !bir.int
// CHECK-NEXT: }
// CHECK-NOT: bir.safepoint

bir.func @no_alloc() -> !bir.int {
  %0 = bir.constant #bir.int<42> : !bir.int
  bir.return %0 : !bir.int
}
