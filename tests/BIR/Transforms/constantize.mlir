// RUN: %bir-opt --constantize %s | %FileCheck %s
// RUN: %bir-opt %s | %FileCheck %s --check-prefix=ROUNDTRIP

// CHECK-LABEL: bir.func @lower_constant
// ROUNDTRIP-LABEL: bir.func @lower_constant
bir.func @lower_constant() -> !bir.int {
    // CHECK-NOT: bir.constant
    // CHECK: arith.constant 42 : i32
    // ROUNDTRIP: bir.constant 42 : !bir.int
    %0 = bir.constant 42 : !bir.int
    bir.return %0 : !bir.int
}
