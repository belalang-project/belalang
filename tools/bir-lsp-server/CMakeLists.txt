add_executable(bir-lsp-server bir-lsp-server.cpp)
target_link_libraries(bir-lsp-server
  PRIVATE
    BelalangBIR
    MLIRGCDialect
    MLIRArithDialect
    MLIRControlFlowDialect
    MLIRFuncDialect
    MLIRLLVMDialect
    MLIRLspServerLib
)
