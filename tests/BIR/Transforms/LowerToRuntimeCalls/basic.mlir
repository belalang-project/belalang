// RUN: %bir-opt --split-input-file --bir-lower-to-runtime-calls %s | %FileCheck %s

// CHECK: bir.func @brt_init()
// CHECK: bir.call @brt_init() : () -> ()
bir.func @main() {
  bir.return
}
