// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK:      define i1 @basic() {
// CHECK-NEXT:   ret i1 true
// CHECK-NEXT: }

bir.func @basic() -> !bir.bool {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  bir.return %0 : !bir.bool
}
