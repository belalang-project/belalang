// RUN: %bir-opt --split-input-file --bir-lower-func-expr %s | %FileCheck %s

// CHECK-LABEL: bir.func private @fn.explicit_return.anon
// CHECK: %{{.*}} = bir.constant #bir.int<42> : !bir.int
// CHECK: bir.return

// CHECK-LABEL: bir.func @explicit_return
// CHECK-NEXT:   bir.return
bir.func @explicit_return() {
  %0 = bir.func_expr : () -> !bir.int {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.return %1 : !bir.int
  }
  bir.return
}

// -----

// CHECK-LABEL: bir.func private @fn.with_args.anon
// CHECK: bir.return

// CHECK-LABEL: bir.func @with_args
// CHECK-NEXT:   bir.return
bir.func @with_args() {
  %0 = bir.func_expr : (!bir.int) -> !bir.int {
  ^bb0(%arg0: !bir.int):
    bir.return %arg0 : !bir.int
  }
  bir.return
}

// -----

// CHECK-LABEL: bir.func private @fn.void_return.anon
// CHECK: bir.return

// CHECK-LABEL: bir.func @void_return
// CHECK-NEXT:   bir.return
bir.func @void_return() {
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  bir.return
}

// -----

// CHECK-LABEL: bir.func private @fn.void_return.anon_0
// CHECK: bir.return

// CHECK-LABEL: bir.func private @fn.void_return.anon
// CHECK: bir.return

// CHECK-LABEL: bir.func @void_return
// CHECK-NEXT:   bir.return
bir.func @void_return() {
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  %1 = bir.func_expr : () -> () {
    bir.return
  }
  bir.return
}

// -----

// Allocations from nested regions are hoisted in source order.
// CHECK-LABEL: bir.func @hoist_alloc_stack
// CHECK-NEXT: %[[FIRST:.*]] = bir.alloc_stack : !bir.ref<!bir.int>
// CHECK-NEXT: %[[SECOND:.*]] = bir.alloc_stack : !bir.ref<!bir.int>
// CHECK-NEXT: bir.scope
bir.func @hoist_alloc_stack() {
  bir.scope {
    %first = bir.alloc_stack : !bir.ref<!bir.int>
    %second = bir.alloc_stack : !bir.ref<!bir.int>
    bir.yield
  }
  bir.return
}
