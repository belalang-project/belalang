macro(belalang_tablegen output input)
  set(LLVM_TARGET_DEFINITIONS "${input}")
  get_filename_component(output_dir "${output}" DIRECTORY)
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${output_dir}")
  mlir_tablegen("${output}" ${ARGN})
endmacro()

belalang_tablegen(belalang/BIR/IR/BIRDialect.h.inc belalang/BIR/IR/BIRDialect.td -gen-dialect-decls)
belalang_tablegen(belalang/BIR/IR/BIRDialect.cpp.inc belalang/BIR/IR/BIRDialect.td -gen-dialect-defs)
belalang_tablegen(belalang/BIR/IR/BIRTypes.h.inc belalang/BIR/IR/BIRTypes.td -gen-typedef-decls)
belalang_tablegen(belalang/BIR/IR/BIRTypes.cpp.inc belalang/BIR/IR/BIRTypes.td -gen-typedef-defs)
belalang_tablegen(belalang/BIR/IR/BIRAttrs.h.inc belalang/BIR/IR/BIRAttrs.td -gen-attrdef-decls)
belalang_tablegen(belalang/BIR/IR/BIRAttrs.cpp.inc belalang/BIR/IR/BIRAttrs.td -gen-attrdef-defs)
belalang_tablegen(belalang/BIR/IR/BIROps.h.inc belalang/BIR/IR/BIROps.td -gen-op-decls)
belalang_tablegen(belalang/BIR/IR/BIROps.cpp.inc belalang/BIR/IR/BIROps.td -gen-op-defs)
belalang_tablegen(belalang/BIR/IR/BIREnumAttrs.h.inc belalang/BIR/IR/BIROps.td -gen-enum-decls)
belalang_tablegen(belalang/BIR/IR/BIREnumAttrs.cpp.inc belalang/BIR/IR/BIROps.td -gen-enum-defs)
belalang_tablegen(belalang/BIR/Interfaces/LoopOpInterface.h.inc belalang/BIR/Interfaces/LoopOpInterface.td -gen-op-interface-decls)
belalang_tablegen(belalang/BIR/Interfaces/LoopOpInterface.cpp.inc belalang/BIR/Interfaces/LoopOpInterface.td -gen-op-interface-defs)
belalang_tablegen(belalang/BIR/Passes.h.inc belalang/BIR/Passes.td -gen-pass-decls)
add_public_tablegen_target(BelalangBIRTableGen)

belalang_tablegen(belalang/GCIR/IR/GCIRDialect.h.inc belalang/GCIR/IR/GCIRDialect.td -gen-dialect-decls)
belalang_tablegen(belalang/GCIR/IR/GCIRDialect.cpp.inc belalang/GCIR/IR/GCIRDialect.td -gen-dialect-defs)
belalang_tablegen(belalang/GCIR/IR/GCIRTypes.h.inc belalang/GCIR/IR/GCIRTypes.td -gen-typedef-decls)
belalang_tablegen(belalang/GCIR/IR/GCIRTypes.cpp.inc belalang/GCIR/IR/GCIRTypes.td -gen-typedef-defs)
add_public_tablegen_target(BelalangGCIRTableGen)

execute_process(
  COMMAND git rev-parse HEAD
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  OUTPUT_VARIABLE BELALANG_GIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)
if(NOT BELALANG_GIT_HASH)
  set(BELALANG_GIT_HASH unknown)
endif()
set(GIT_HASH "${BELALANG_GIT_HASH}")
configure_file(
  belalang/Version.h.inc.in
  "${BELALANG_GENERATED_INCLUDE_DIR}/belalang/Version.h.inc"
  @ONLY
)

add_library(BelalangHeaders INTERFACE)
target_include_directories(BelalangHeaders INTERFACE
  "$<BUILD_INTERFACE:${BELALANG_SOURCE_INCLUDE_DIR}>"
  "$<BUILD_INTERFACE:${BELALANG_GENERATED_INCLUDE_DIR}>"
)
