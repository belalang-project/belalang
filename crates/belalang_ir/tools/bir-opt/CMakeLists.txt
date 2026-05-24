get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(extension_libs GLOBAL PROPERTY MLIR_EXTENSION_LIBS)

set(LIBS
  ${dialect_libs}
  ${extension_libs}
  MLIRArithDialect
  MLIROptLib
  BelalangIR
)

add_llvm_executable(bir-opt bir-opt.cpp)
target_link_libraries(bir-opt PRIVATE ${LIBS})
