// RUN: %bir-opt --split-input-file --bir-optimize-struct-layout %s \
// RUN: | %FileCheck %s

// CHECK: !struct_Compact = !bir.struct<"Compact", {!bir.bool, !bir.int}, reorder = [1, 0]>
// CHECK: bir.func @compact
bir.func @compact(%0: !bir.struct<"Compact", {!bir.bool, !bir.int}>) {
  bir.return
}

// -----

// CHECK: !struct_CompactMember = !bir.struct<"CompactMember", {!bir.bool, !bir.int}, reorder = [1, 0]>
// CHECK: bir.get_member %{{.*}}[0]
bir.func @compact_member(%0: !bir.ref<!bir.struct<"CompactMember", {!bir.bool, !bir.int}>>) {
  bir.get_member %0[0] { name = "flag" }
      : !bir.ref<!bir.struct<"CompactMember", {!bir.bool, !bir.int}>> -> !bir.ref<!bir.bool>
  bir.return
}
