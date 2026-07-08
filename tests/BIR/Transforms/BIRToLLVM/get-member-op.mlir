// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

bir.func @f(%0: !bir.ref<!bir.struct<"T", {!bir.int}>>) {
  // CHECK: llvm.getelementptr inbounds|nuw %{{.*}}[0, 0] : (!llvm.ptr) -> !llvm.ptr, !llvm.struct<"T", (i64)>
  bir.get_member %0[0] { name = "x" }
      : !bir.ref<!bir.struct<"T", {!bir.int}>> -> !bir.ref<!bir.int>
  bir.return
}
