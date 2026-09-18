// RUN: %bir-opt --split-input-file --convert-bir-to-llvm %s | %FileCheck %s

// CHECK: llvm.mlir.global private constant @struct.[[H:.*]]([42, 41]) {addr_space = 0 : i32} : !llvm.struct<"Point", (i64, i64)>

// CHECK-LABEL: llvm.func @f() -> !llvm.struct<"Point", (i64, i64)>
// CHECK: %[[C0:.*]] = llvm.mlir.addressof @struct.[[H]] : !llvm.ptr
// CHECK: %[[C1:.*]] = llvm.load %[[C0]] : !llvm.ptr -> !llvm.struct<"Point", (i64, i64)>
// CHECK: llvm.return %[[C1]] : !llvm.struct<"Point", (i64, i64)>

bir.func @f() -> !bir.struct<"Point", {!bir.int, !bir.int}> {
  %0 = bir.constant #bir.struct<{#bir.int<42> : !bir.int, #bir.int<41> : !bir.int}> : !bir.struct<"Point", {!bir.int, !bir.int}>
  bir.return %0 : !bir.struct<"Point", {!bir.int, !bir.int}>
}

// -----

// CHECK: llvm.mlir.global private constant @struct.[[H:.*]]([]) {addr_space = 0 : i32} : !llvm.struct<"Empty", ()>

// CHECK-LABEL: llvm.func @f() -> !llvm.struct<"Empty", ()>
// CHECK: %[[C0:.*]] = llvm.mlir.addressof @struct.[[H]] : !llvm.ptr
// CHECK: %[[C1:.*]] = llvm.load %[[C0]] : !llvm.ptr -> !llvm.struct<"Empty", ()>
// CHECK: llvm.return %[[C1]] : !llvm.struct<"Empty", ()>

bir.func @f() -> !bir.struct<"Empty", {}> {
  %0 = bir.constant #bir.struct<{}> : !bir.struct<"Empty", {}>
  bir.return %0 : !bir.struct<"Empty", {}>
}
