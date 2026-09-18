// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK-LABEL: llvm.func @f() -> !llvm.ptr
// CHECK: llvm.call @brt_gc_alloc_layout
// CHECK: llvm.getelementptr inbounds|nuw {{.*}}[0, 0]
// CHECK: llvm.store {{.*}} : i64, !llvm.ptr
// CHECK: llvm.getelementptr inbounds|nuw {{.*}}[0, 1]
// CHECK: llvm.store {{.*}} : i64, !llvm.ptr
// CHECK: llvm.return {{.*}} : !llvm.ptr

bir.func @f() -> !bir.array<[!bir.int, !bir.int]> {
  %0 = bir.constant #bir.array<[#bir.int<42> : !bir.int, #bir.int<41> : !bir.int]> : !bir.array<[!bir.int, !bir.int]>
  bir.return %0 : !bir.array<[!bir.int, !bir.int]>
}

// -----

// CHECK: llvm.func @f() -> !llvm.ptr
// CHECK: llvm.call @brt_gc_alloc_layout
// CHECK: llvm.store {{.*}} : i64, !llvm.ptr
// CHECK: llvm.store {{.*}} : f64, !llvm.ptr
// CHECK: llvm.return {{.*}} : !llvm.ptr

bir.func @f() -> !bir.array<[!bir.int, !bir.float]> {
  %0 = bir.constant #bir.array<[#bir.int<42> : !bir.int, #bir.float<4.0> : !bir.float]> : !bir.array<[!bir.int, !bir.float]>
  bir.return %0 : !bir.array<[!bir.int, !bir.float]>
}
