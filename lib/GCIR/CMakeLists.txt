add_library(BelalangGCIR)
add_dependencies(BelalangGCIR
  BelalangGCIRIncGen
  BelalangGCIRConversionPassIncGen
  BelalangGCIRTransformsPassIncGen
)
target_link_libraries(BelalangGCIR
  PUBLIC
    MLIRIR
    MLIRFuncToLLVM
    MLIRLLVMDialect
    MLIRPass
)

add_subdirectory(Conversions)
add_subdirectory(IR)
add_subdirectory(Transforms)
