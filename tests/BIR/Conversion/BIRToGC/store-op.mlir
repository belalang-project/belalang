// RUN: %bir-opt --convert-bir-to-gc --split-input-file %s | %FileCheck %s

// CHECK-LABEL: bir.func @main
// CHECK-SAME:      (%[[VAL:.*]]: !bir.int, %[[PTR:.*]]: !bir.ref<!bir.int>)
// CHECK-NEXT:    %[[CAST:.*]] = builtin.unrealized_conversion_cast %[[PTR]] : !bir.ref<!bir.int> to !gc.ptr<!bir.int>
// CHECK-NEXT:    gc.store %[[VAL]], %[[CAST]] : !gc.ptr<!bir.int>
// CHECK-NEXT:    bir.return

bir.func @main(%val : !bir.int, %ptr : !bir.ref<!bir.int>) {
  bir.store %val to %ptr : !bir.int to !bir.ref<!bir.int>
  bir.return
}
