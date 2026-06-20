// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

bir.func @explicit_return() {
  // CHECK: %[[C0:.*]] = bir.func_expr : () -> !bir.int
  %0 = bir.func_expr : () -> !bir.int {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.return %1 : !bir.int
  }
  bir.return
}

// -----

bir.func @with_arguments() {
  // CHECK: %[[C0:.*]] = bir.func_expr : (!bir.int) -> !bir.int
  %0 = bir.func_expr : (!bir.int) -> !bir.int {
  ^bb0(%arg0: !bir.int):
    bir.return %arg0 : !bir.int
  }
  bir.return
}

// -----

bir.func @void_return_types() {
  // CHECK: %[[C0:.*]] = bir.func_expr : () -> ()
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  bir.return
}

// -----

bir.func @implicit_return_on_empty_region() {
  %0 = bir.func_expr : () -> () {}
  bir.return
}

// -----

bir.func @implicit_return() {
  %0 = bir.func_expr : () -> () {
    %1 = bir.constant #bir.int<42> : !bir.int
  }
  bir.return
}

// -----

bir.func @types_mismatch() {
  // expected-error@+1 {{returned types do not match function signature types}}
  %0 = bir.func_expr : () -> !bir.float {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.return %1 : !bir.int
  }
  bir.return
}
