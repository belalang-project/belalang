// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --split-input-file --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK-DAG: %bel.String = type { ptr, i64 }
// CHECK-DAG: @str.[[H:.*]] = private constant [5 x i8] c"hello"
// CHECK-DAG: declare ptr @brt_gc_alloc_layout(i64, i64, ptr)
// CHECK-DAG: declare void @brt_init()
// CHECK-DAG: @llvm.global_ctors {{.*}} ptr @ctor

// CHECK-LABEL:define %bel.String @main() {
// CHECK-NEXT:   %[[C1:.*]] = call ptr @brt_gc_alloc_layout(i64 16, i64 0, ptr null)
// CHECK-NEXT:   store %bel.String { ptr @str.[[H]], i64 5 }, ptr %[[C1]], align 8
// CHECK-NEXT:   %[[C2:.*]] = load %bel.String, ptr %[[C1]], align 8
// CHECK-NEXT:   ret %bel.String %[[C2]]
// CHECK-NEXT: }

bir.func @main() -> !bir.string {
  // x := "hello"
  %0 = bir.constant #bir.string<"hello"> : !bir.string
  %1 = bir.declare "x" : !bir.ref<!bir.string>
  bir.store %0 to %1 : !bir.string to !bir.ref<!bir.string>

  // return x
  %2 = bir.load %1 : (!bir.ref<!bir.string>) -> !bir.string
  bir.return %2 : !bir.string
}

// -----

// CHECK-DAG: %bel.String = type { ptr, i64 }
// CHECK-DAG: @str.[[H:.*]] = private constant [5 x i8] c"hello"
// CHECK-DAG: declare ptr @brt_gc_alloc_layout(i64, i64, ptr)
// CHECK-DAG: declare void @brt_print_string(%bel.String)
// CHECK-DAG: declare void @brt_init()
// CHECK-DAG: @llvm.global_ctors {{.*}} ptr @ctor

// CHECK-LABEL:define void @main() {
// CHECK-NEXT:   %[[C1:.*]] = call ptr @brt_gc_alloc_layout(i64 16, i64 0, ptr null)
// CHECK-NEXT:   store %bel.String { ptr @str.[[H]], i64 5 }, ptr %[[C1]], align 8
// CHECK-NEXT:   %[[C2:.*]] = load %bel.String, ptr %[[C1]], align 8
// CHECK-NEXT:   call void @brt_print_string(%bel.String %[[C2]])
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

bir.func private @brt_print_string(!bir.string)

bir.func @main() {
  // x := "hello"
  %0 = bir.constant #bir.string<"hello"> : !bir.string
  %1 = bir.declare "x" : !bir.ref<!bir.string>
  bir.store %0 to %1 : !bir.string to !bir.ref<!bir.string>

  // print(x)
  %2 = bir.load %1 : (!bir.ref<!bir.string>) -> !bir.string
  bir.call @brt_print_string(%2) : (!bir.string) -> ()
  bir.return
}
