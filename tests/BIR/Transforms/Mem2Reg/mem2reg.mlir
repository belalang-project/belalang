// RUN: %bir-opt --split-input-file --mem2reg %s | %FileCheck %s

// CHECK-LABEL: bir.func @straight
// CHECK-NEXT:  %[[VALUE:.*]] = bir.constant #bir.int<42> : !bir.int
// CHECK-NEXT:  call @use(%[[VALUE]]) : (!bir.int) -> ()
// CHECK-NEXT:  bir.return
bir.func private @use(!bir.int)

bir.func @straight() {
  %0 = bir.alloc_stack : !bir.ref<!bir.int>
  %1 = bir.constant #bir.int<42> : !bir.int
  bir.store %1 to %0 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.call @use(%2) : (!bir.int) -> ()
  bir.return
}

// -----

// CHECK-LABEL: bir.func @default_load
// CHECK-NEXT:  %[[ZERO:.*]] = bir.constant #bir.int<0> : !bir.int
// CHECK-NEXT:  call @use(%[[ZERO]]) : (!bir.int) -> ()
// CHECK-NEXT:  bir.return
bir.func private @use(!bir.int)

bir.func @default_load() {
  %0 = bir.alloc_stack : !bir.ref<!bir.int>
  %1 = bir.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.call @use(%1) : (!bir.int) -> ()
  bir.return
}

// -----

// CHECK-LABEL: bir.func @heap_not_promoted
// CHECK-NEXT:  %[[HEAP:.*]] = bir.alloc_heap : !bir.ref<!bir.int>
// CHECK-NEXT:  %[[VALUE:.*]] = bir.constant #bir.int<7> : !bir.int
// CHECK-NEXT:  bir.store %[[VALUE]] to %[[HEAP]] : !bir.int to !bir.ref<!bir.int>
// CHECK-NEXT:  %[[LOAD:.*]] = bir.load %[[HEAP]] : (!bir.ref<!bir.int>) -> !bir.int
// CHECK-NEXT:  call @use(%[[LOAD]]) : (!bir.int) -> ()
// CHECK-NEXT:  bir.return
bir.func private @use(!bir.int)

bir.func @heap_not_promoted() {
  %0 = bir.alloc_heap : !bir.ref<!bir.int>
  %1 = bir.constant #bir.int<7> : !bir.int
  bir.store %1 to %0 : !bir.int to !bir.ref<!bir.int>
  %2 = bir.load %0 : (!bir.ref<!bir.int>) -> !bir.int
  bir.call @use(%2) : (!bir.int) -> ()
  bir.return
}

// -----

// CHECK-LABEL: bir.func @blocked_get_member
// CHECK:       bir.alloc_stack
// CHECK:       bir.get_member
// CHECK:       bir.store
bir.func @blocked_get_member() {
  %0 = bir.alloc_stack : !bir.ref<!bir.struct<"T", {!bir.int}>>
  %1 = bir.get_member %0[0] {name = "x"} : !bir.ref<!bir.struct<"T", {!bir.int}>> -> !bir.ref<!bir.int>
  %2 = bir.constant #bir.int<1> : !bir.int
  bir.store %2 to %1 : !bir.int to !bir.ref<!bir.int>
  bir.return
}
