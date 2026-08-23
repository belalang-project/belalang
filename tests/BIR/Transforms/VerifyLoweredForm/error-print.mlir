// RUN: %not %bir-opt --split-input-file --bir-verify-lowered-form %s 2>&1 | %FileCheck %s

// CHECK: 'bir.print' op unexpected high-level BIR operation after lowering pipeline
bir.func @has_print() {
  %0 = bir.constant #bir.int<1> : !bir.int
  bir.print %0 : !bir.int
  bir.return
}
