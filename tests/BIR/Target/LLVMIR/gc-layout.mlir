// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --split-input-file --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK-DAG: @gc.ptr_offsets.{{.*}} = private constant {{\[1 x i32\]}} {{\[i32 8\]}}
// CHECK-DAG: declare ptr @brt_gc_alloc_layout(i64, i64, ptr)
// CHECK-LABEL: define void @main() {
// CHECK: call ptr @brt_gc_alloc_layout(i64 16, i64 1, ptr @gc.ptr_offsets.{{.*}})
// CHECK: ret void

bir.func @main() {
  %0 = bir.alloc_heap : !bir.ref<!bir.struct<"Box", {!bir.int, !bir.ref<!bir.int>}>>
  bir.return
}
