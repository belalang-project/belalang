// RUN: %bir-opt --split-input-file --bir-lower-func-expr %s | %FileCheck %s

// CHECK-LABEL: bir.func @fn.explicit_return.anon
// CHECK: %{{.*}} = bir.constant #bir.int<42> : !bir.int
// CHECK: bir.return

// CHECK-LABEL: bir.func @explicit_return
// CHECK-NEXT:   %{{.*}} = bir.constant #bir.fn<@fn.explicit_return.anon> : () -> !bir.int
// CHECK-NEXT:   bir.return
bir.func @explicit_return() {
  %0 = bir.func_expr : () -> !bir.int {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.return %1 : !bir.int
  }
  bir.return
}

// -----

// CHECK-LABEL: bir.func @fn.with_args.anon
// CHECK: bir.return

// CHECK-LABEL: bir.func @with_args
// CHECK-NEXT:   %{{.*}} = bir.constant #bir.fn<@fn.with_args.anon> : (!bir.int) -> !bir.int
// CHECK-NEXT:   bir.return
bir.func @with_args() {
  %0 = bir.func_expr : (!bir.int) -> !bir.int {
  ^bb0(%arg0: !bir.int):
    bir.return %arg0 : !bir.int
  }
  bir.return
}

// -----

// CHECK-LABEL: bir.func @fn.void_return.anon
// CHECK: bir.return

// CHECK-LABEL: bir.func @void_return
// CHECK-NEXT:   %{{.*}} = bir.constant #bir.fn<@fn.void_return.anon> : () -> ()
// CHECK-NEXT:   bir.return
bir.func @void_return() {
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  bir.return
}

// -----

// CHECK-LABEL: bir.func @fn.void_return.anon_0
// CHECK: bir.return

// CHECK-LABEL: bir.func @fn.void_return.anon
// CHECK: bir.return

// CHECK-LABEL: bir.func @void_return
// CHECK-NEXT:   %{{.*}} = bir.constant #bir.fn<@fn.void_return.anon> : () -> ()
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
