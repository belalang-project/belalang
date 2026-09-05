set(LLVM_TARGET_DEFINITIONS BIRDialect.td)
mlir_tablegen(BIRDialect.h.inc -gen-dialect-decls)
mlir_tablegen(BIRDialect.cpp.inc -gen-dialect-defs)

set(LLVM_TARGET_DEFINITIONS BIRTypes.td)
mlir_tablegen(BIRTypes.h.inc -gen-typedef-decls)
mlir_tablegen(BIRTypes.cpp.inc -gen-typedef-defs)

set(LLVM_TARGET_DEFINITIONS BIRAttrs.td)
mlir_tablegen(BIRAttrs.h.inc -gen-attrdef-decls)
mlir_tablegen(BIRAttrs.cpp.inc -gen-attrdef-defs)

set(LLVM_TARGET_DEFINITIONS BIROps.td)
mlir_tablegen(BIROps.h.inc -gen-op-decls)
mlir_tablegen(BIROps.cpp.inc -gen-op-defs)
mlir_tablegen(BIREnumAttrs.h.inc -gen-enum-decls)
mlir_tablegen(BIREnumAttrs.cpp.inc -gen-enum-defs)

add_mlir_dialect_tablegen_target(BelalangBIRIncGen)
