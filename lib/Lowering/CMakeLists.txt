add_library(BelalangLowering Pipelines.cpp)
target_link_libraries(BelalangLowering
  PUBLIC
    BelalangBIR
    MLIRGCDialect
    MLIRArithToLLVM
    MLIRConvertToLLVMPass
    MLIRReconcileUnrealizedCasts
)
