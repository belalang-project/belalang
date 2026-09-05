set(LLVM_TARGET_DEFINITIONS GCIRDialect.td)
mlir_tablegen(GCIRDialect.h.inc -gen-dialect-decls)
mlir_tablegen(GCIRDialect.cpp.inc -gen-dialect-defs)

set(LLVM_TARGET_DEFINITIONS GCIRTypes.td)
mlir_tablegen(GCIRTypes.h.inc -gen-typedef-decls)
mlir_tablegen(GCIRTypes.cpp.inc -gen-typedef-defs)

add_mlir_dialect_tablegen_target(BelalangGCIRIncGen)
