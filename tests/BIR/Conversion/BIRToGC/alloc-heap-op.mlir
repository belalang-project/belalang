// RUN: %bir-opt --split-input-file --convert-bir-to-gc %s | %FileCheck %s

// CHECK-LABEL: bir.func @main() {
// CHECK-NEXT:    %[[MEM:.*]] = gc.alloc : !gc.ptr<!bir.int> <size = 8, pointer_offsets = []>
// CHECK-NEXT:    bir.return
// CHECK-NEXT:  }

bir.func @main() {
  %0 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.return
}

// -----

// CHECK-LABEL: bir.func @roots(
// CHECK-SAME:      %[[ROOT:.*]]: !bir.ref<!bir.int>)
// CHECK-NEXT:    %[[ROOT_CAST:.*]] = builtin.unrealized_conversion_cast
// CHECK-SAME:      [[ROOT]] : !bir.ref<!bir.int> to !gc.ptr<!bir.int>
// CHECK:         %[[OBJECT:.*]], %[[RELOCATED:.*]] = gc.alloc roots(%[[ROOT_CAST]] : !gc.ptr<!bir.int>) : !gc.ptr<!bir.int>
// CHECK-SAME:      <size = 8, pointer_offsets = []>

bir.func @roots(%root : !bir.ref<!bir.int>) {
  %object, %relocated = bir.alloc_heap : !bir.ref<!bir.int>
      roots(%root : !bir.ref<!bir.int>)
  bir.return
}

// -----

// CHECK-LABEL: bir.func @main() {
// CHECK-NEXT:    %[[MEM:.*]] = gc.alloca : !gc.ptr<!bir.int>
// CHECK-NEXT:    bir.return
// CHECK-NEXT:  }

bir.func @main() {
  %0 = bir.alloc_stack : !bir.ref<!bir.int>
  bir.return
}
