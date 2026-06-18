// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// -----

// CHECK-LABEL: bir.func @basic
bir.func @basic() -> !bir.int {
    // CHECK: bir.constant #bir.int<42> : !bir.int
    %0 = bir.constant #bir.int<42> : !bir.int
    bir.return %0 : !bir.int
}
