// RUN: %bir-opt --split-input-file --bir-lower-decl-to-memory %s \
// RUN: | %FileCheck %s

// CHECK-LABEL: bir.func @main
bir.func @main() {
  // CHECK:      %[[VALUE:.*]] = bir.constant #bir.string<"hello"> : !bir.string
  // CHECK-NEXT: %[[MEM:.*]] = bir.alloc_stack : !bir.ref<!bir.string>
  // CHECK-NEXT: bir.store %[[VALUE]] to %[[MEM]] : !bir.string to !bir.ref<!bir.string>
  %0 = bir.constant #bir.string<"hello"> : !bir.string
  %1 = bir.declare "x" : !bir.ref<!bir.string>
  bir.store %0 to %1 : !bir.string to !bir.ref<!bir.string>
  bir.return
}
