// RUN: %bir-opt --split-input-file --convert-gcir-to-llvm %s | %FileCheck %s
// RUN: %bir-opt --split-input-file --convert-to-llvm %s | %FileCheck %s

module attributes {
  gc.runtime.alloc = "allocate",
  gc.runtime.push_roots = "push_roots",
  gc.runtime.pop_roots = "pop_roots"
} {
  // CHECK-LABEL: llvm.func @allocate(i64, i64, !llvm.ptr) -> !llvm.ptr
  // CHECK-LABEL: llvm.func @main
  // CHECK:         %[[SIZE:.*]] = llvm.mlir.constant(8 : i64) : i64
  // CHECK:         %[[COUNT:.*]] = llvm.mlir.constant(0 : i64) : i64
  // CHECK:         %[[OFFSETS:.*]] = llvm.mlir.zero : !llvm.ptr
  // CHECK:         %[[OBJECT:.*]] = llvm.call @allocate(%[[SIZE]], %[[COUNT]], %[[OFFSETS]])
  // CHECK:         llvm.return %[[OBJECT]] : !llvm.ptr
  func.func @main() -> !gc.ptr<i64> {
    %object = gc.alloc : !gc.ptr<i64> {
      size = 8 : i64,
      pointer_offsets = array<i32>
    }
    return %object : !gc.ptr<i64>
  }
}

// -----

module attributes {
  gc.runtime.alloc = "allocate",
  gc.runtime.push_roots = "push_roots",
  gc.runtime.pop_roots = "pop_roots"
} {
  // CHECK-LABEL: llvm.func @rooted
  // CHECK:         %[[ROOTS:.*]] = llvm.alloca
  // CHECK:         %[[SLOT:.*]] = llvm.alloca
  // CHECK:         llvm.store %{{.*}}, %[[SLOT]]
  // CHECK:         llvm.call @push_roots
  // CHECK:         %[[OBJECT:.*]] = llvm.call @allocate
  // CHECK-NEXT:    llvm.call @pop_roots
  // CHECK-NEXT:    %[[RELOCATED:.*]] = llvm.load %[[SLOT]]
  // CHECK:         %[[WITH_OBJECT:.*]] = llvm.insertvalue %[[OBJECT]], %{{.*}}[0]
  // CHECK-NEXT:    %[[RESULTS:.*]] = llvm.insertvalue %[[RELOCATED]], %[[WITH_OBJECT]][1]
  // CHECK-NEXT:    llvm.return %[[RESULTS]]
  func.func @rooted(%root : !gc.ptr<i64>)
      -> (!gc.ptr<i64>, !gc.ptr<i64>) {
    %object, %relocated = gc.alloc
        roots(%root : !gc.ptr<i64>) : !gc.ptr<i64> {
      size = 8 : i64,
      pointer_offsets = array<i32>
    }
    return %object, %relocated : !gc.ptr<i64>, !gc.ptr<i64>
  }
}
