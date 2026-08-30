// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK-LABEL: llvm.func @f
// CHECK: llvm.getelementptr inbounds|nuw %{{.*}}[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<(i64, f64)>

bir.func @f(%0: !bir.array<[!bir.int, !bir.float]>) {
  %1 = bir.get_element %0[0]
      : !bir.array<[!bir.int, !bir.float]> -> !bir.ref<!bir.int>
  bir.return
}
