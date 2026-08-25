// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s | %FileCheck %s
// RUN: %bir-opt --split-input-file --bir-lowering-pipeline=enable-dce=false %s | %FileCheck --check-prefix=NODCE %s

// CHECK-LABEL: bir.func @drops_dead_pure_work
// CHECK-NOT: bir.add
// CHECK-NOT: bir.mul
// CHECK: bir.safepoint 0
// CHECK-NEXT: %[[HEAP:.*]] = bir.alloc_heap : !bir.ref<!bir.int>
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

// CHECK-NOT: @fn.drops_unused_func_expr.anon

// CHECK-LABEL: bir.func @drops_unused_func_expr
// CHECK-NEXT: bir.return

// NODCE-LABEL: bir.func private @fn.drops_unused_func_expr.anon
// NODCE-LABEL: bir.func @drops_unused_func_expr
// NODCE-NEXT: bir.return

bir.func @drops_unused_func_expr() {
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  bir.return
}

// -----

// CHECK-LABEL: bir.func private @fn.keeps_referenced_func_expr.anon
// CHECK-LABEL: bir.func @keeps_referenced_func_expr
// CHECK: bir.constant #bir.fn<@fn.keeps_referenced_func_expr.anon> : () -> ()
// CHECK: bir.call_indirect
bir.func @keeps_referenced_func_expr() {
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  bir.call_indirect %0() : () -> ()
  bir.return
}
