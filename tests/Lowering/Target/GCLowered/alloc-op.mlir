// RUN: %bir-opt --bir-lowering-pipeline=target=gc-lowered %s | %FileCheck %s

// CHECK-LABEL: bir.func @main
// CHECK-NEXT:    %[[OBJ:.*]] = gc.call @malloc() : () -> !gc.ptr<!bir.int>
// CHECK-NEXT:    bir.return

bir.func @main() -> !bir.ref<!bir.int> {
  %object = bir.alloc_heap : !bir.ref<!bir.int>
  bir.return
}
