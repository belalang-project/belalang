// RUN: %bir-opt --allow-unregistered-dialect %s | %FileCheck %s

// CHECK: !gc.derived_ptr<i64>
// CHECK: !gc.derived_ptr<!bir.int>
module {
  %int = "test.source"() : () -> !gc.derived_ptr<i64>
  "test.sink"(%int) : (!gc.derived_ptr<i64>) -> ()

  %bir_int = "test.source"() : () -> !gc.derived_ptr<!bir.int>
  "test.sink"(%bir_int) : (!gc.derived_ptr<!bir.int>) -> ()
}
