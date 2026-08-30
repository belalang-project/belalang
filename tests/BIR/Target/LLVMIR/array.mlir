// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK: define ptr @array() {
// CHECK: call ptr @brt_gc_alloc_layout(i64 16, i64 0, ptr null)
// CHECK: store i64 42
// CHECK: store i64 41
// CHECK: ret ptr

bir.func @array() -> !bir.array<[!bir.int, !bir.int]> {
  %0 = bir.constant #bir.array<[#bir.int<42> : !bir.int, #bir.int<41> : !bir.int]> : !bir.array<[!bir.int, !bir.int]>
  bir.return %0 : !bir.array<[!bir.int, !bir.int]>
}
