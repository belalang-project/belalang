// RUN: not %bir-opt --convert-gcir-to-llvm %s 2>&1 | %FileCheck %s

// CHECK: error: 'gc.alloc' op requires 'size' and 'pointer_offsets' attributes for LLVM lowering

module {
  func.func @test() -> !gc.ptr<i64> {
    %p = "gc.alloc"() : () -> !gc.ptr<i64>
    return %p : !gc.ptr<i64>
  }
}
