// RUN: %bir-opt --split-input-file --trivial-dce --symbol-dce %s | %FileCheck %s

// CHECK-LABEL: bir.func @drops_dead_pure_work
// CHECK-NOT: bir.add
// CHECK-NOT: bir.mul
// CHECK: %[[HEAP:.*]] = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK: bir.return

bir.func @drops_dead_pure_work() {
  %0 = bir.constant #bir.int<1> : !bir.int
  %1 = bir.constant #bir.int<2> : !bir.int
  %2 = bir.add %0, %1 : !bir.int
  %3 = bir.mul %2, %1 : !bir.int
  %4 = bir.alloc_heap : !bir.ref<!bir.int>
  bir.return
}

// -----

// CHECK-LABEL: bir.func @drops_unused_func_expr
// CHECK-NEXT:    %0 = bir.func_expr : () -> () {
// CHECK-NEXT:    }
// CHECK-NEXT:    bir.return

bir.func @drops_unused_func_expr() {
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  bir.return
}

// -----

// CHECK-LABEL: bir.func @keeps_referenced_func_expr()
// CHECK-NEXT:    %0 = bir.func_expr : () -> () {
// CHECK-NEXT:    }
// CHECK-NEXT:    bir.call_indirect %0() : () -> ()
// CHECK-NEXT:    bir.return

bir.func @keeps_referenced_func_expr() {
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  bir.call_indirect %0() : () -> ()
  bir.return
}
