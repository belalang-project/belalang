// RUN: %bir-opt --allow-unregistered-dialect %s | %FileCheck %s

// CHECK: !gc.ptr<i64>
// CHECK: !gc.ptr<!bir.int>
module {
  %int = "test.source"() : () -> !gc.ptr<i64>
  "test.sink"(%int) : (!gc.ptr<i64>) -> ()

  %bir_int = "test.source"() : () -> !gc.ptr<!bir.int>
  "test.sink"(%bir_int) : (!gc.ptr<!bir.int>) -> ()
}
