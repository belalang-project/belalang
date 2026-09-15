set(LLVM_TARGET_DEFINITIONS Passes.td)
mlir_tablegen(Passes.h.inc -gen-pass-decls)
add_public_tablegen_target(BelalangBIRPassIncGen)

add_subdirectory(IR)
add_subdirectory(Interfaces)
add_subdirectory(Transforms)
add_subdirectory(Conversions)
