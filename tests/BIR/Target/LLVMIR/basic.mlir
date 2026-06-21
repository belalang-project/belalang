// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK:      define i64 @basic() {
// CHECK-NEXT:   ret i64 42
// CHECK-NEXT: }

bir.func @basic() -> !bir.int {
  %0 = bir.constant #bir.int<42> : !bir.int
  bir.return %0 : !bir.int
}
