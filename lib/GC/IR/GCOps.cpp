#include "mlir/Dialect/GC/IR/GC.h"

#define GET_OP_CLASSES
#include "mlir/Dialect/GC/IR/GCOps.cpp.inc"

namespace mlir {
namespace gc {

llvm::SmallVector<mlir::MemorySlot> AllocaOp::getPromotableSlots() {
  auto ptrType = mlir::cast<PtrType>(getResult().getType());
  return {mlir::MemorySlot{getResult(), ptrType.getPointee()}};
}

mlir::Value AllocaOp::getDefaultValue(const mlir::MemorySlot &slot,
                                      mlir::OpBuilder &builder) {
  // Like llvm.alloca, gc.alloca provides uninitialized storage.
  return {};
}

void AllocaOp::handleBlockArgument(const mlir::MemorySlot &slot,
                                   mlir::BlockArgument argument,
                                   mlir::OpBuilder &builder) {}

std::optional<mlir::PromotableAllocationOpInterface>
AllocaOp::handlePromotionComplete(const mlir::MemorySlot &slot,
                                  mlir::Value defaultValue,
                                  mlir::OpBuilder &builder) {
  erase();
  return std::nullopt;
}

} // namespace gc
} // namespace mlir
