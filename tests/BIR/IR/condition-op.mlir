// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

bir.func @main() {
  bir.while {
    %cond = bir.constant #bir.bool<true> : !bir.bool
    // CHECK: bir.condition
    bir.condition %cond
  } do {
    bir.continue
  }
  bir.return
}

// -----

bir.func @main() {
  %cond = bir.constant #bir.bool<true> : !bir.bool
  // expected-error@+1 {{must be within a conditional region}}
  bir.condition %cond
}
