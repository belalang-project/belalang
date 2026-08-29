// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s | %FileCheck %s

// CHECK-LABEL: bir.func @merges_duplicate_pure_work
// CHECK: %[[SUM:.*]] = bir.add
// CHECK-NOT: bir.add
// CHECK: bir.call @use(%[[SUM]])
// CHECK: bir.call @use(%[[SUM]])
// CHECK: bir.return
bir.func @use(!bir.int)

bir.func @merges_duplicate_pure_work() {
  %0 = bir.constant #bir.int<1> : !bir.int
  %1 = bir.constant #bir.int<2> : !bir.int
  %2 = bir.add %0, %1 : !bir.int
  %3 = bir.add %0, %1 : !bir.int
  bir.call @use(%2) : (!bir.int) -> ()
  bir.call @use(%3) : (!bir.int) -> ()
  bir.return
}
