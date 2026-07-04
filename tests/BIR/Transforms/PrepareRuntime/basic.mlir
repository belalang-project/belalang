// RUN: %bir-opt --split-input-file --bir-prepare-runtime %s | %FileCheck %s

// CHECK: bir.func @brt_init()
// CHECK: bir.call @brt_init() : () -> ()
bir.func @main() {
  bir.return
}
