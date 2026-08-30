// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

bir.func private @use(!bir.int)

bir.func @main() {
  // CHECK: bir.while
  bir.while {
    %cond = bir.constant #bir.bool<true> : !bir.bool
    bir.condition %cond
  } do {
    %v = bir.constant #bir.int<1> : !bir.int
    bir.call @use(%v) : (!bir.int) -> ()
    bir.continue
  }
  bir.return
}
