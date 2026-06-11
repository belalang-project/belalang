// RUN: %bir-opt --split-input-file --runtimize %s | %FileCheck %s

// CHECK: bir.func private @brt_print_int(!bir.int)

// CHECK-LABEL: bir.func @print_belalang
bir.func @print_belalang() {
  // CHECK: %[[C0:.*]] = bir.constant 0 : !bir.int
  // CHECK-NEXT: call @brt_print_int(%[[C0]]) : (!bir.int) -> ()
  %0 = bir.constant 0 : !bir.int
  bir.print %0 : !bir.int
  bir.return
}

// -----

// CHECK: bir.func private @brt_print_float(!bir.float)

// CHECK-LABEL: bir.func @print_belalang
bir.func @print_belalang() {
  // CHECK: %[[C0:.*]] = bir.constant 3.000000e+00 : !bir.float
  // CHECK-NEXT: call @brt_print_float(%[[C0]]) : (!bir.float) -> ()
  %0 = bir.constant 3.0 : !bir.float
  bir.print %0 : !bir.float
  bir.return
}
