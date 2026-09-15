#ifndef BELALANG_BIR_PASSES_H_
#define BELALANG_BIR_PASSES_H_

#include "mlir/Pass/Pass.h"

namespace belalang {
namespace bir {

#define GEN_PASS_DECL
#include "belalang/BIR/Passes.h.inc"

#define GEN_PASS_REGISTRATION
#include "belalang/BIR/Passes.h.inc"

} // namespace bir
} // namespace belalang

#endif // BELALANG_BIR_PASSES_H_
