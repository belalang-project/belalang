#ifndef MLIR_DIALECT_GC_PASSES_H_
#define MLIR_DIALECT_GC_PASSES_H_

#include "mlir/Pass/Pass.h"

namespace mlir {
class DialectRegistry;

#define GEN_PASS_DECL
#include "mlir/Dialect/GC/Passes.h.inc"

namespace gc {
#define GEN_PASS_REGISTRATION
#include "mlir/Dialect/GC/Passes.h.inc"

void registerGCToLLVMInterface(mlir::DialectRegistry &registry);
} // namespace gc

} // namespace mlir

#endif // MLIR_DIALECT_GC_PASSES_H_
