// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s | %FileCheck %s

// CHECK-LABEL: bir.func @pipeline
// CHECK-NOT: bir.declare
// CHECK-NOT: bir.print
// CHECK-NOT: bir.if
bir.func @pipeline() {
  %0 = bir.constant #bir.int<1> : !bir.int
  %1 = bir.declare "x" : !bir.ref<!bir.int>
  bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
  bir.print %2 : !bir.int
  bir.return
}
