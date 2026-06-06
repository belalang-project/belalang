// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK-LABEL: func.func @basic
func.func @basic() -> !bir.int {
    %0 = bir.constant 4 : !bir.int
    %1 = bir.constant 2 : !bir.int
    %2 = bir.add %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %3 = bir.sub %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %4 = bir.mul %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %5 = bir.div %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %6 = bir.mod %0, %1 : (!bir.int, !bir.int) -> !bir.int
    return %0 : !bir.int
}
