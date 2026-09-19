add_library(MLIRGCDialect)
add_dependencies(MLIRGCDialect
  MLIRGCIncGen
  MLIRGCPassIncGen
)
target_link_libraries(MLIRGCDialect
  PUBLIC
    MLIRConvertToLLVMInterface
    MLIRIR
    MLIRFuncToLLVM
    MLIRLLVMDialect
    MLIRPass
)

add_subdirectory(Analysis)
add_subdirectory(Conversions)
add_subdirectory(IR)
add_subdirectory(Transforms)
