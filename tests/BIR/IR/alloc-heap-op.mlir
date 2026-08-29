// RUN: %bir-opt --verify-roundtrip %s | %FileCheck %s

// CHECK-LABEL: bir.func @rooted_alloc
// CHECK:       %[[ROOT:.*]] = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK:       %[[OTHER:.*]] = bir.alloc_heap : !bir.ref<!bir.float>
// CHECK:       %[[OBJECT:.*]], %[[ROOTS:.*]]:2 = bir.alloc_heap : !bir.ref<!bir.bool> roots(%[[ROOT]], %[[OTHER]] : !bir.ref<!bir.int>, !bir.ref<!bir.float>)
// CHECK:       bir.load %[[ROOTS]]#0 : (!bir.ref<!bir.int>) -> !bir.int
bir.func @rooted_alloc() -> !bir.int {
  %root = bir.alloc_heap : !bir.ref<!bir.int>
  %other = bir.alloc_heap : !bir.ref<!bir.float>
  %object, %root_next, %other_next = bir.alloc_heap : !bir.ref<!bir.bool> roots(%root, %other : !bir.ref<!bir.int>, !bir.ref<!bir.float>)
  %value = bir.load %root_next : (!bir.ref<!bir.int>) -> !bir.int
  bir.return %value : !bir.int
}
