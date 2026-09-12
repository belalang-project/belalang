#ifndef MLIR_DIALECT_GC_PASSES_H_
#define MLIR_DIALECT_GC_PASSES_H_

#include "mlir/Pass/Pass.h"

namespace mlir {

#define GEN_PASS_DECL
#include "mlir/Dialect/GC/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "mlir/Dialect/GC/Passes.h.inc"

} // namespace mlir

#endif // MLIR_DIALECT_GC_PASSES_H_
