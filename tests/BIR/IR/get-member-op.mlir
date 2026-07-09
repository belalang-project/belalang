// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

// CHECK: bir.func @f(%[[ARG0:.*]]: !bir.ref<!struct_T>)
bir.func @f(%0: !bir.ref<!bir.struct<"T", {!bir.int}>>) {
  // CHECK: bir.get_member %[[ARG0]][0] {name = "x"} : !bir.ref<!struct_T> -> !bir.ref<!bir.int>
  bir.get_member %0[0] { name = "x" }
      : !bir.ref<!bir.struct<"T", {!bir.int}>> -> !bir.ref<!bir.int>
  bir.return
}
