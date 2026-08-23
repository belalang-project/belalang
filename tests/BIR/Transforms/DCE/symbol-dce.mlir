// RUN: %bir-opt --split-input-file --symbol-dce %s | %FileCheck %s

// CHECK-NOT: @dead
bir.func private @dead() {
  bir.return
}

// CHECK-LABEL: bir.func @main
bir.func @main() {
  bir.return
}

// -----

// CHECK-LABEL: bir.func private @kept
// CHECK-LABEL: bir.func @main
// CHECK: bir.constant #bir.fn<@kept> : () -> ()
bir.func private @kept() {
  bir.return
}

bir.func @main() {
  %0 = bir.constant #bir.fn<@kept> : () -> ()
  bir.return
}
