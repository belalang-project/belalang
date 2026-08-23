// RUN: %not %bir-opt --split-input-file --bir-verify-lowered-form %s 2>&1 | %FileCheck %s

// CHECK: 'bir.declare' op unexpected high-level BIR operation after lowering pipeline
bir.func @has_declare() {
  %0 = bir.declare "x" : !bir.ref<!bir.int>
  bir.return
}
