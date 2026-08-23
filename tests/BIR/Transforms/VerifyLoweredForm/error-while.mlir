// RUN: %not %bir-opt --split-input-file --bir-verify-lowered-form %s 2>&1 | %FileCheck %s

// CHECK: 'bir.while' op unexpected high-level BIR operation after lowering pipeline
bir.func @has_while() {
  bir.while {
    %0 = bir.constant #bir.bool<false> : !bir.bool
    bir.condition %0
  } do {
    bir.yield
  }
  bir.return
}
