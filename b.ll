; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

%bel.String = type { ptr, i64 }

@str.10760714692547961661 = private constant [5 x i8] c"hello"

declare ptr @brt_gc_alloc_layout(i64, i64, ptr)

define %bel.String @main() {
  %1 = call ptr @brt_gc_alloc_layout(i64 16, i64 0, ptr null)
  store %bel.String { ptr @str.10760714692547961661, i64 5 }, ptr %1, align 8
  %2 = load %bel.String, ptr %1, align 8
  ret %bel.String %2
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}

; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"

%bel.String = type { ptr, i64 }

@str.10760714692547961661 = private constant [5 x i8] c"hello"

declare ptr @brt_gc_alloc_layout(i64, i64, ptr)

declare void @brt_print_string(%bel.String)

define void @main() {
  %1 = call ptr @brt_gc_alloc_layout(i64 16, i64 0, ptr null)
  store %bel.String { ptr @str.10760714692547961661, i64 5 }, ptr %1, align 8
  %2 = load %bel.String, ptr %1, align 8
  call void @brt_print_string(%bel.String %2)
  ret void
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
