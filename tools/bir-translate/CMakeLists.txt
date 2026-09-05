add_executable(bir-translate bir-translate.cpp)
target_link_libraries(bir-translate
  PRIVATE
    BelalangBIR
    MLIRBuiltinToLLVMIRTranslation
    MLIRLLVMDialect
    MLIRLLVMToLLVMIRTranslation
    MLIRTranslateLib
)
