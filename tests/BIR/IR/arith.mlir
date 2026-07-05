// RUN: %bir-opt --split-input-file --verify-roundtrip %s | %FileCheck %s

// CHECK-LABEL: bir.func @basic
bir.func @basic() -> !bir.int {
    %0 = bir.constant #bir.int<4> : !bir.int
    %1 = bir.constant #bir.int<2> : !bir.int
    %2 = bir.add %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %3 = bir.sub %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %4 = bir.mul %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %5 = bir.div %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %6 = bir.mod %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %7 = bir.and %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %8 = bir.or %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %9 = bir.xor %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %10 = bir.shl %0, %1 : (!bir.int, !bir.int) -> !bir.int
    %11 = bir.shr %0, %1 : (!bir.int, !bir.int) -> !bir.int
    bir.return %0 : !bir.int
}
