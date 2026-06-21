// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK: declare void @brt_print_float(double)
// CHECK: declare void @brt_print_bool(i1)

//      CHECK: define void @print_belalang() {
// CHECK-NEXT:   call void @brt_print_float(double 3.000000e+00)
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

//      CHECK: define void @print_bool() {
// CHECK-NEXT:   call void @brt_print_bool(i1 true)
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

bir.func @print_belalang() {
  %0 = bir.constant #bir.float<3.0> : !bir.float
  bir.print %0 : !bir.float
  bir.return
}

bir.func @print_bool() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  bir.print %0 : !bir.bool
  bir.return
}
