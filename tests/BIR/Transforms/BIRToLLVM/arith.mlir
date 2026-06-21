// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK: module {
// CHECK-NEXT: llvm.func @basic() -> i64 {

bir.func @basic() -> !bir.int {
  // CHECK-NEXT: %0 = llvm.mlir.constant(4 : i64) : i64
  %0 = bir.constant #bir.int<4> : !bir.int

  // CHECK-NEXT: %1 = llvm.mlir.constant(2 : i64) : i64
  %1 = bir.constant #bir.int<2> : !bir.int

  // CHECK-NEXT: %2 = llvm.add %0, %1 : i64
  %2 = bir.add %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: %3 = llvm.sub %0, %1 : i64
  %3 = bir.sub %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: %4 = llvm.mul %0, %1 : i64
  %4 = bir.mul %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: %5 = llvm.sdiv %0, %1 : i64
  %5 = bir.div %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: %6 = llvm.srem %0, %1 : i64
  %6 = bir.mod %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: llvm.return %0 : i64
  bir.return %0 : !bir.int
}

// CHECK-NEXT: }
// CHECK-NEXT: }
