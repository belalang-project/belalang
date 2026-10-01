// RUN: %not %bir-opt --split-input-file --convert-gc-to-llvm %s 2>&1 | %FileCheck %s
// RUN: %not %bir-opt --split-input-file --convert-to-llvm %s 2>&1 | %FileCheck %s

// CHECK: error: 'gc.alloc' op requires 'size' and 'pointer_offsets' attributes for LLVM lowering

module {
  func.func @test() -> !gc.ptr<i64> {
    %p = "gc.alloc"() : () -> !gc.ptr<i64>
    return %p : !gc.ptr<i64>
  }
}

// -----

// CHECK: error: 'gc.alloc' op requires GC runtime symbol configuration on the parent module

func.func @test() -> !gc.ptr<i64> {
  %p = gc.alloc : !gc.ptr<i64> {
    size = 8 : i64,
    pointer_offsets = array<i32>
  }
  return %p : !gc.ptr<i64>
}
