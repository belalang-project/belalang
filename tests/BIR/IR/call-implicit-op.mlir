// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

bir.func @void_arguments() {
  // CHECK: %[[C0:.*]] = bir.func_expr : () -> !bir.int
  %0 = bir.func_expr : () -> !bir.int {
    %1 = bir.constant #bir.int<42> : !bir.int
    bir.return %1 : !bir.int
  }
  // CHECK: %[[C1:.*]] = bir.call_indirect %[[C0]]() : () -> !bir.int
  %1 = bir.call_indirect %0() : () -> !bir.int
  bir.return
}

// -----

bir.func @argument_types_differs() {
  // expected-note@+1 {{prior use here}}
  %0 = bir.func_expr : (!bir.int) -> !bir.int {
  ^bb0(%arg0: !bir.int):
    bir.return %arg0 : !bir.int
  }
  %1 = bir.constant #bir.float<42.0> : !bir.float
  // expected-error@+1 {{expects different type than prior uses: '(!bir.float) -> !bir.int' vs '(!bir.int) -> !bir.int'}}
  %1 = bir.call_indirect %0(%1) : (!bir.float) -> !bir.int
  bir.return
}
