add_library(BelalangBIR)
add_dependencies(BelalangBIR
  BelalangBIRIncGen
  BelalangBIRLoopOpInterfaceIncGen
  BelalangBIRPassIncGen
)
target_link_libraries(BelalangBIR PUBLIC
  MLIRAnalysis
  MLIRControlFlowDialect
  MLIRControlFlowToLLVM
  MLIRFuncDialect
  MLIRIR
  MLIRLLVMDialect
  MLIRMemorySlotInterfaces
  MLIRPass
  MLIRSupport
  MLIRTransformUtils
  MLIRTransforms
)

add_subdirectory(Analysis)
add_subdirectory(IR)
add_subdirectory(Interfaces)
add_subdirectory(Transforms)
