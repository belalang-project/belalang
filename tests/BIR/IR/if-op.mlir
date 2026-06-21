// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  // CHECK:      bir.if %{{.*}} {
  // CHECK-NEXT: }
  bir.if %0 {}
  bir.return
}

// -----

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  %2 = bir.if %0 {
    %1 = bir.constant #bir.int<1> : !bir.int
    bir.yield %1 : !bir.int
  } : !bir.int
  // expected-error@-1 {{should also have an else region}}
  bir.return
}

// -----

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  // CHECK:      bir.if %{{.*}} {
  // CHECK-NEXT:   bir.yield
  // CHECK-NEXT: } else {
  // CHECK-NEXT:   bir.yield
  // CHECK-NEXT: }
  bir.if %0 {
    bir.yield
  } else {
    bir.yield
  }
  bir.return
}

// -----

bir.func @main() -> !bir.int {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  %3 = bir.if %0 {
    %1 = bir.constant #bir.int<1> : !bir.int
    bir.yield %1 : !bir.int
  } else {
    %2 = bir.constant #bir.int<2> : !bir.int
    bir.yield %2 : !bir.int
  } : !bir.int
  bir.return %3 : !bir.int
}

// -----

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  // expected-error@+1 {{region #0 should have no arguments}}
  bir.if %0 {
  ^bb0(%arg0 : !bir.int):
    bir.yield
  } else {
  ^bb0(%arg0 : !bir.int):
    bir.yield
  }
  bir.return
}

// -----

bir.func @main() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  // expected-error@+1 {{region #1 should have no arguments}}
  bir.if %0 {
    bir.yield
  } else {
  ^bb0(%arg0 : !bir.int):
    bir.yield
  }
  bir.return
}
