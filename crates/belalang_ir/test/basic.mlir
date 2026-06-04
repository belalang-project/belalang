// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK-LABEL: func.func @basic
func.func @basic(%lhs: i32, %rhs: i32) -> i32 {
    // CHECK: bir.minimal
    bir.minimal
    return %lhs : i32
}

// -----

// CHECK-LABEL: func.func @basic
func.func @basic() -> i32 {
    // CHECK: bir.constant 42 : i32
    %0 = bir.constant 42 : i32
    return %0 : i32
}
