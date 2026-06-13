// RUN: %bir-opt --split-input-file --runtimize --bir-to-llvm %s \
// RUN: | %bir-translate --mlir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK: declare void @brt_print_float(float)

//      CHECK: define void @print_belalang() {
// CHECK-NEXT:   call void @brt_print_float(float 3.000000e+00)
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

bir.func @print_belalang() {
  %0 = bir.constant 3.0 : !bir.float
  bir.print %0 : !bir.float
  bir.return
}
