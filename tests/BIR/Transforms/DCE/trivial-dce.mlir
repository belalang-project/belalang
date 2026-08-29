// RUN: %bir-opt --split-input-file --trivial-dce %s | %FileCheck %s

// CHECK-LABEL: bir.func @unused_arithmetic
// CHECK-NEXT: bir.return
bir.func @unused_arithmetic() {
  %0 = bir.constant #bir.int<1> : !bir.int
  %1 = bir.constant #bir.int<2> : !bir.int
  %2 = bir.add %0, %1 : !bir.int
  %3 = bir.mul %2, %1 : !bir.int
  %4 = bir.sub %3, %0 : !bir.int
  %5 = bir.div %4, %1 : !bir.int
  %6 = bir.mod %5, %1 : !bir.int
  bir.return
}

// -----

// CHECK-LABEL: bir.func @unused_constant
// CHECK-NEXT: bir.return
bir.func @unused_constant() {
  %0 = bir.constant #bir.int<42> : !bir.int
  bir.return
}

// -----

// CHECK-LABEL: bir.func @unused_get_member
// CHECK-NEXT: %[[REF:.*]] = bir.alloc_stack : !bir.ref<!struct_Point>
// CHECK-NEXT: bir.return
bir.func @unused_get_member() {
  %0 = bir.alloc_stack : !bir.ref<!bir.struct<"Point", {!bir.int, !bir.int}>>
  %1 = bir.get_member %0[0] {name = "x"} : !bir.ref<!bir.struct<"Point", {!bir.int, !bir.int}>> -> !bir.ref<!bir.int>
  bir.return
}

// -----

// CHECK-LABEL: bir.func @retain_effectful_ops
// CHECK: %[[ZERO:.*]] = bir.constant #bir.int<0> : !bir.int
// CHECK: %[[STACK:.*]] = bir.alloc_stack : !bir.ref<!bir.int>
// CHECK: bir.store %[[ZERO]] to %[[STACK]] : !bir.int to !bir.ref<!bir.int>
// CHECK: bir.load %[[STACK]] : (!bir.ref<!bir.int>) -> !bir.int
// CHECK: call @use(%[[ZERO]]) : (!bir.int) -> ()
// CHECK: bir.call @callee(%[[ZERO]]) : (!bir.int) -> ()
// CHECK: %[[HEAP:.*]], %[[RELOCATED:.*]] = bir.alloc_heap : !bir.ref<!bir.int> roots(%[[STACK]] : !bir.ref<!bir.int>)
// CHECK: bir.return
bir.func @callee(!bir.int)

bir.func @use(!bir.int)

bir.func @retain_effectful_ops() {
  %0 = bir.constant #bir.int<0> : !bir.int
  %1 = bir.alloc_stack : !bir.ref<!bir.int>
  bir.store %0 to %1 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.load %1 : (!bir.ref<!bir.int>) -> !bir.int
  bir.call @use(%0) : (!bir.int) -> ()
  bir.call @callee(%0) : (!bir.int) -> ()
  %3, %4 = bir.alloc_heap : !bir.ref<!bir.int> roots(%1 : !bir.ref<!bir.int>)
  bir.return
}

// -----

// CHECK-LABEL: bir.func @unused_bitwise_and_cmp
// CHECK-NEXT: bir.return
bir.func @unused_bitwise_and_cmp() {
  %0 = bir.constant #bir.int<1> : !bir.int
  %1 = bir.constant #bir.int<2> : !bir.int
  %2 = bir.and %0, %1 : (!bir.int, !bir.int) -> !bir.int
  %3 = bir.or %2, %1 : (!bir.int, !bir.int) -> !bir.int
  %4 = bir.xor %3, %0 : (!bir.int, !bir.int) -> !bir.int
  %5 = bir.shl %4, %0 : (!bir.int, !bir.int) -> !bir.int
  %6 = bir.shr %5, %0 : (!bir.int, !bir.int) -> !bir.int
  %7 = bir.cmp eq %6, %1 : !bir.int
  bir.return
}

// -----

// CHECK-LABEL: bir.func @unreachable_blocks
// CHECK-NEXT: bir.return
// CHECK-NEXT: }
bir.func @use(!bir.int)

bir.func @unreachable_blocks() {
  bir.return
^dead:
  %0 = bir.constant #bir.int<1> : !bir.int
  bir.call @use(%0) : (!bir.int) -> ()
  bir.return
}
