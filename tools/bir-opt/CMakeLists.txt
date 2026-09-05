add_executable(bir-opt bir-opt.cpp)
target_link_libraries(bir-opt
  PRIVATE
    BelalangBIR
    BelalangGCIR
    MLIRLLVMDialect
    MLIROptLib
)
