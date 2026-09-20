// RUN: %bir-opt --split-input-file --bir-lowering-pipeline=target=gc %s | %FileCheck %s

// CHECK-LABEL: bir.func @main
// CHECK-SAME:      (%[[PTR:.*]]: !bir.ref<!bir.int>)
// CHECK-NEXT:    %[[CAST:.*]] = builtin.unrealized_conversion_cast %[[PTR]] : !bir.ref<!bir.int> to !gc.ptr<!bir.int>
// CHECK-NEXT:    gc.load %[[CAST]] : !gc.ptr<!bir.int>
// CHECK-NEXT:    bir.return

bir.func @main(%ptr : !bir.ref<!bir.int>) {
  bir.load %ptr : (!bir.ref<!bir.int>) -> !bir.int
  bir.return
}
