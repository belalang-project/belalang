#ifndef BELALANG_GCIR_TRANSFORMS_PASSES_H_
#define BELALANG_GCIR_TRANSFORMS_PASSES_H_

#include "mlir/Pass/Pass.h"
#include <memory>

namespace mlir {

#define GEN_PASS_DECL
#include "belalang/GCIR/Transforms/Passes.h.inc"

void registerGCIRPrepareGCSafepointsPass();

} // namespace mlir

#endif // BELALANG_GCIR_TRANSFORMS_PASSES_H_
