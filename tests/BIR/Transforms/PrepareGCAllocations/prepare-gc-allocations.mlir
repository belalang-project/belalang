// RUN: %bir-opt --split-input-file --bir-prepare-gc-allocations %s | %FileCheck %s

// CHECK-LABEL: bir.func @alloc_straight_line
// CHECK:       %[[FIRST:.*]] = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK:       %[[SECOND:.*]], %[[FIRST_NEXT:.*]] = bir.alloc_heap : !bir.ref<!bir.float> roots(%[[FIRST]] : !bir.ref<!bir.int>)
// CHECK:       bir.load %[[FIRST_NEXT]] : (!bir.ref<!bir.int>) -> !bir.int
// CHECK:       %[[THIRD:.*]] = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK-NOT:   roots
bir.func @alloc_straight_line() -> !bir.int {
  %0 = bir.constant #bir.int<1> : !bir.int
  %1 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.constant #bir.float<1.000000e+00> : !bir.float
  %3 = bir.alloc_heap : !bir.ref<!bir.float>
  bir.store %2 to %3 : !bir.float to !bir.ref<!bir.float>
  %4 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
  %5 = bir.constant #bir.int<1> : !bir.int
  %6 = bir.add %4, %5 : !bir.int
  %7 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.store %6 to %7 : !bir.int to !bir.ref<!bir.int>
  %8 = bir.load %7 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %8 : !bir.int
}

// -----

// CHECK-LABEL: bir.func @alloc_cross_block
// CHECK:       %[[FIRST:.*]] = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK:       cf.br ^bb1
// CHECK:       ^bb1:
// CHECK:       bir.load %[[FIRST]] : (!bir.ref<!bir.int>) -> !bir.int
// CHECK:       bir.alloc_heap : !bir.ref<!bir.int>
bir.func @alloc_cross_block() -> !bir.int {
  %0 = bir.constant #bir.int<0> : !bir.int
  %1 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
  cf.br ^bb1
^bb1:
  %2 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
  %3 = bir.constant #bir.int<1> : !bir.int
  %4 = bir.add %2, %3 : !bir.int
  %5 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.store %4 to %5 : !bir.int to !bir.ref<!bir.int>
  %6 = bir.load %5 : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %6 : !bir.int
}

// -----

// CHECK-LABEL: bir.func @no_alloc
// CHECK-NOT:   bir.alloc_heap
bir.func @no_alloc() -> !bir.int {
  %0 = bir.constant #bir.int<42> : !bir.int
  bir.return %0 : !bir.int
}
