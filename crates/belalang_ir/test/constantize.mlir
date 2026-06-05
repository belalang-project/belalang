// RUN: %bir-opt --constantize %s | %FileCheck %s
// RUN: %bir-opt %s | %FileCheck %s --check-prefix=ROUNDTRIP

// CHECK-LABEL: func.func @lower_constant
// ROUNDTRIP-LABEL: func.func @lower_constant
func.func @lower_constant() -> i32 {
    // CHECK-NOT: bir.constant
    // CHECK: arith.constant 42 : i32
    // ROUNDTRIP: bir.constant 42 : i32
    %0 = bir.constant 42 : i32
    return %0 : i32
}
