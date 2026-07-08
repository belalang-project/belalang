// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK: llvm.func @f(%arg0: !llvm.struct<"T", (i64, i64)>)
bir.func @f(%0: !bir.struct<"T", {!bir.int, !bir.int}>) {
  bir.return
}

// -----

// CHECK: llvm.func @f(%arg0: !llvm.struct<"T", ()>)
bir.func @f(%0: !bir.struct<"T", {}>) {
  bir.return
}
