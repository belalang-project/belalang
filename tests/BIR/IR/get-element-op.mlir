// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

// CHECK: bir.func @f(%[[ARRAY:.*]]: !bir.array<[!bir.int, !bir.float]>)
// CHECK: bir.get_element %[[ARRAY]][0] : !bir.array<[!bir.int, !bir.float]> -> !bir.ref<!bir.int>
bir.func @f(%0: !bir.array<[!bir.int, !bir.float]>) {
  %1 = bir.get_element %0[0]
      : !bir.array<[!bir.int, !bir.float]> -> !bir.ref<!bir.int>
  bir.return
}

// -----

// expected-error@+2 {{element index is out of bounds}}
bir.func @out_of_bounds(%0: !bir.array<[!bir.int]>) {
  %1 = bir.get_element %0[1]
      : !bir.array<[!bir.int]> -> !bir.ref<!bir.int>
  bir.return
}
