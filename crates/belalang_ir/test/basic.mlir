// RUN: %mlir-opt --verify-roundtrip %s | %FileCheck %s

// CHECK: func.func @simple_arith
func.func @simple_arith(%lhs: i32, %rhs: i32) -> i32 {
    %sum = arith.addi %lhs, %rhs : i32
    return %sum : i32
}
