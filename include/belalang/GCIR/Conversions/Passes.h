#ifndef BELALANG_GCIR_CONVERSIONS_PASSES_H_
#define BELALANG_GCIR_CONVERSIONS_PASSES_H_

#include "mlir/Pass/Pass.h"
#include <memory>

namespace mlir {

#define GEN_PASS_DECL
#include "belalang/GCIR/Conversions/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "belalang/GCIR/Conversions/Passes.h.inc"

} // namespace mlir

#endif // BELALANG_GCIR_CONVERSIONS_PASSES_H_
