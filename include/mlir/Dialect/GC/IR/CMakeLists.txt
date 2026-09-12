set(LLVM_TARGET_DEFINITIONS GCDialect.td)
mlir_tablegen(GCDialect.h.inc -gen-dialect-decls)
mlir_tablegen(GCDialect.cpp.inc -gen-dialect-defs)

set(LLVM_TARGET_DEFINITIONS GCTypes.td)
mlir_tablegen(GCTypes.h.inc -gen-typedef-decls)
mlir_tablegen(GCTypes.cpp.inc -gen-typedef-defs)

set(LLVM_TARGET_DEFINITIONS GCOps.td)
mlir_tablegen(GCOps.h.inc -gen-op-decls)
mlir_tablegen(GCOps.cpp.inc -gen-op-defs)

add_mlir_dialect_tablegen_target(MLIRGCIncGen)
