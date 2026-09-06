set(LLVM_TARGET_DEFINITIONS LoopOpInterface.td)
mlir_tablegen(LoopOpInterface.h.inc -gen-op-interface-decls)
mlir_tablegen(LoopOpInterface.cpp.inc -gen-op-interface-defs)
add_mlir_generic_tablegen_target(BelalangBIRLoopOpInterfaceIncGen)
