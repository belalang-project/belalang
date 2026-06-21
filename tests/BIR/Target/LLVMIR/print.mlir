// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK: declare void @brt_print_float(double)

//      CHECK: define void @print_belalang() {
// CHECK-NEXT:   call void @brt_print_float(double 3.000000e+00)
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

bir.func @print_belalang() {
  %0 = bir.constant #bir.float<3.0> : !bir.float
  bir.print %0 : !bir.float
  bir.return
}
