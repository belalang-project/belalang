// RUN: %bir-opt --split-input-file --verify-roundtrip --verify-diagnostics %s | %FileCheck %s

// CHECK: !struct_T = !bir.struct<"T", {!bir.int, !bir.int}>
// CHECK: bir.func @f(%{{.*}}: !struct_T)
bir.func @f(%0: !bir.struct<"T", {!bir.int, !bir.int}>) {
  bir.return
}

// -----

// CHECK: !struct_T = !bir.struct<"T", {}>
// CHECK: bir.func @f(%{{.*}}: !struct_T)
bir.func @f(%0: !bir.struct<"T", {}>) {
  bir.return
}
