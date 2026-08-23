// RUN: %not %bir-opt --split-input-file --bir-verify-lowered-form %s 2>&1 | %FileCheck %s

// CHECK: 'bir.func_expr' op unexpected high-level BIR operation after lowering pipeline
bir.func @has_func_expr() {
  %0 = bir.func_expr : () -> () {
    bir.return
  }
  bir.return
}
