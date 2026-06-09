// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// -----

// CHECK-LABEL: func.func @basic
func.func @basic() -> !bir.int {
    // CHECK: bir.constant 42 : !bir.int
    %0 = bir.constant 42 : !bir.int
    return %0 : !bir.int
}
