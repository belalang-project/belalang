// RUN: %bir-opt --split-input-file --bir-lowering-pipeline=target=gc %s | %FileCheck %s

// CHECK-LABEL: bir.func @main() {
// CHECK-NEXT:    %[[MEM:.*]] = gc.alloc : !gc.ptr<!bir.int>
// CHECK-NEXT:    bir.return
// CHECK-NEXT:  }

bir.func @main() {
  %0 = bir.alloc_heap : !bir.ref<!bir.int>
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
