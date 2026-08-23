// RUN: %not %bir-opt --split-input-file --bir-verify-lowered-form %s 2>&1 | %FileCheck %s

// CHECK: 'bir.if' op unexpected high-level BIR operation after lowering pipeline
bir.func @has_if() {
  %0 = bir.constant #bir.bool<true> : !bir.bool
  bir.if %0 {
    bir.yield
  }
  bir.return
}
