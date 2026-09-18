#ifndef BELALANG_CONVERSIONS_PASSES_H_
#define BELALANG_CONVERSIONS_PASSES_H_

#include "mlir/Dialect/ControlFlow/IR/ControlFlow.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Pass/Pass.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/ADT/SmallVector.h"

#include <cstdint>
#include <optional>

namespace belalang {
namespace bir {

struct GCAllocationLayout {
  uint64_t size;
  llvm::SmallVector<uint32_t> pointerOffsets;
};

#define GEN_PASS_DECL
#include "belalang/BIR/Conversions/Passes.h.inc"

void populateBelalangBIRToLLVMPatterns(mlir::RewritePatternSet &patterns,
                                       mlir::TypeConverter &typeConverter);

void registerBIRToLLVMInterface(mlir::DialectRegistry &registry);

std::optional<GCAllocationLayout> getGCAllocationLayout(mlir::Type referentType,
                                                        mlir::Operation *scope);

} // namespace bir
} // namespace belalang

#endif // BELALANG_CONVERSIONS_PASSES_H_
