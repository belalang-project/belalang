// RUN: %bir-opt --verify-roundtrip %s | %FileCheck %s

// CHECK: func.func @basic
func.func @basic(%lhs: i32, %rhs: i32) -> i32 {
    // CHECK: bir.minimal
    bir.minimal
    return %lhs : i32
}
