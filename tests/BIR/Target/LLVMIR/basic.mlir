// RUN: %bir-opt --split-input-file --bir-to-llvm %s \
// RUN: | %bir-translate --mlir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK:      define i32 @basic() {
// CHECK-NEXT:   ret i32 42
// CHECK-NEXT: }

bir.func @basic() -> !bir.int {
  %0 = bir.constant 42 : !bir.int
  bir.return %0 : !bir.int
}
