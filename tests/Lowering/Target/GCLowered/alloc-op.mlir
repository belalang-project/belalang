// RUN: %bir-opt --bir-lowering-pipeline=target=gc-lowered %s | %FileCheck %s

// CHECK-LABEL: bir.func @main
// CHECK-NEXT:    %[[OBJ:.*]] = gc.call @malloc() : () -> !gc.ptr<!bir.int>
// CHECK-NEXT:    %[[VALUE:.*]] = bir.constant #bir.int<0> : !bir.int
// CHECK-NEXT:    gc.store %[[VALUE]], %[[OBJ]] : !gc.ptr<!bir.int>
// CHECK-NEXT:    bir.return

bir.func @main() {
  %object = bir.alloc_heap : !bir.ref<!bir.int>
  %value = bir.constant #bir.int<0> : !bir.int
  bir.store %value to %object : !bir.int to !bir.ref<!bir.int>
  bir.return
}
