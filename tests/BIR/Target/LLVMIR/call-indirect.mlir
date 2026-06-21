// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK:      define i64 @fn.callee() {
// CHECK-NEXT:   ret i64 42
// CHECK-NEXT: }

// CHECK:      define void @caller() {
// CHECK:        call i64 @fn.callee()
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

bir.func @fn.callee() -> !bir.int {
  %0 = bir.constant #bir.int<42> : !bir.int
  bir.return %0 : !bir.int
}

bir.func @caller() {
  %0 = bir.constant #bir.fn<@fn.callee> : () -> !bir.int
  %1 = bir.call_indirect %0() : () -> !bir.int
  bir.return
}
