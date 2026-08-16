// RUN: %bir-opt --split-input-file --bir-lower-decl-to-memory %s \
// RUN: | %FileCheck %s

// CHECK-LABEL: bir.func @main
bir.func @main() -> !bir.string {
  // CHECK:      %[[VALUE:.*]] = bir.constant #bir.string<"hello"> : !bir.string
  // CHECK-NEXT: %[[MEM:.*]] = bir.alloc_heap : !bir.ref<!bir.string>
  // CHECK-NEXT: bir.store %[[VALUE]] to %[[MEM]] : !bir.string to !bir.ref<!bir.string>
  %0 = bir.constant #bir.string<"hello"> : !bir.string
  %1 = bir.declare "x" : !bir.ref<!bir.string>
  bir.store %0 to %1 : !bir.string to !bir.ref<!bir.string>

  // CHECK:      %[[RET:.*]] = bir.load %[[MEM]] : (!bir.ref<!bir.string>) -> !bir.string
  // CHECK-NEXT: bir.return %[[RET]] : !bir.string
  %2 = bir.load %1 : (!bir.ref<!bir.string>) -> !bir.string
  bir.return %2 : !bir.string
}
