// RUN: %bir-opt --split-input-file --bir-lowering-pipeline %s \
// RUN: | %bir-translate --split-input-file --bir-to-llvmir \
// RUN: | %FileCheck %s

// CHECK: @str.[[H:.*]] = private constant [5 x i8] c"hello"

// CHECK: declare void @brt_init()

// CHECK: declare ptr @brt_gc_alloc(i64)

// CHECK:      define { ptr, i64 } @main() {
// CHECK-NEXT:   call void @brt_init()
// CHECK-NEXT:   %[[C1:.*]] = call ptr @brt_gc_alloc(i64 16)
// CHECK-NEXT:   store { ptr, i64 } { ptr @str.[[H]], i64 5 }, ptr %[[C1]], align 8
// CHECK-NEXT:   %[[C2:.*]] = load { ptr, i64 }, ptr %[[C1]], align 8
// CHECK-NEXT:   ret { ptr, i64 } %[[C2]]
// CHECK-NEXT: }

bir.func @main() -> !bir.string {
  // x := "hello"
  %0 = bir.constant #bir.string<"hello"> : !bir.string
  %1 = bir.var.declare "x" : !bir.ref<!bir.string>
  bir.var.store %0 to %1 : !bir.string to !bir.ref<!bir.string>

  // return x
  %2 = bir.var.load %1 : (!bir.ref<!bir.string>) -> !bir.string
  bir.return %2 : !bir.string
}

// -----

// CHECK: @str.[[H:.*]] = private constant [5 x i8] c"hello"

// CHECK: declare void @brt_init()

// CHECK: declare ptr @brt_gc_alloc(i64)

// CHECK: declare void @brt_print_string({ ptr, i64 })

// CHECK:      define void @main() {
// CHECK-NEXT:   call void @brt_init()
// CHECK-NEXT:   %[[C1:.*]] = call ptr @brt_gc_alloc(i64 16)
// CHECK-NEXT:   store { ptr, i64 } { ptr @str.[[H]], i64 5 }, ptr %[[C1]], align 8
// CHECK-NEXT:   %[[C2:.*]] = load { ptr, i64 }, ptr %[[C1]], align 8
// CHECK-NEXT:   call void @brt_print_string({ ptr, i64 } %[[C2]])
// CHECK-NEXT:   ret void
// CHECK-NEXT: }

bir.func @main() {
  // x := "hello"
  %0 = bir.constant #bir.string<"hello"> : !bir.string
  %1 = bir.var.declare "x" : !bir.ref<!bir.string>
  bir.var.store %0 to %1 : !bir.string to !bir.ref<!bir.string>

  // print(x)
  %2 = bir.var.load %1 : (!bir.ref<!bir.string>) -> !bir.string
  bir.print %2 : !bir.string
  bir.return
}
