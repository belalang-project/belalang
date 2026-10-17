add_library(BelalangLLVMGen LLVMGen.cpp)
llvm_map_components_to_libnames(BELALANG_NATIVE_TARGET_LIBS native)
target_link_libraries(BelalangLLVMGen
  PUBLIC
    BelalangBIR
    BelalangLowering
    MLIRArithToLLVM
    LLVMCodeGen
    LLVMCore
    LLVMInstrumentation
    LLVMipo
    LLVMMC
    LLVMPasses
    LLVMSupport
    LLVMTarget
    LLVMTargetParser
    ${BELALANG_NATIVE_TARGET_LIBS}
    MLIRBuiltinToLLVMIRTranslation
    MLIRIR
    MLIRLLVMDialect
    MLIRLLVMToLLVMIRTranslation
    MLIRPass
    MLIRTargetLLVMIRExport
)
