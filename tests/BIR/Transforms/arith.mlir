// RUN: %bir-opt --split-input-file --bir-to-llvm %s | %FileCheck %s

// CHECK: module {
// CHECK-NEXT: llvm.func @basic() -> i32 {

bir.func @basic() -> !bir.int {
  // CHECK-NEXT: %0 = llvm.mlir.constant(4 : i32) : i32
  %0 = bir.constant 4 : !bir.int

  // CHECK-NEXT: %1 = llvm.mlir.constant(2 : i32) : i32
  %1 = bir.constant 2 : !bir.int

  // CHECK-NEXT: %2 = llvm.add %0, %1 : i32
  %2 = bir.add %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: %3 = llvm.sub %0, %1 : i32
  %3 = bir.sub %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: %4 = llvm.mul %0, %1 : i32
  %4 = bir.mul %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: %5 = llvm.sdiv %0, %1 : i32
  %5 = bir.div %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: %6 = llvm.srem %0, %1 : i32
  %6 = bir.mod %0, %1 : (!bir.int, !bir.int) -> !bir.int

  // CHECK-NEXT: llvm.return %0 : i32
  bir.return %0 : !bir.int
}

// CHECK-NEXT: }
// CHECK-NEXT: }
