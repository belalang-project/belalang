#include "mlir/Dialect/GC/IR/GC.h"

#define GET_OP_CLASSES
#include "mlir/Dialect/GC/IR/GCOps.cpp.inc"

namespace mlir {
namespace gc {

// -----------------------------------------------------------------------------
// AllocOp: SafepointOpInterface
// -----------------------------------------------------------------------------

FailureOr<SafepointOpInterface>
AllocOp::rebuildWithRoots(RewriterBase &rewriter, ValueRange roots) {
  OpBuilder::InsertionGuard guard(rewriter);
  rewriter.setInsertionPoint(getOperation());

  SmallVector<Type> resultTypes;
  resultTypes.reserve(roots.size() + 1);

  resultTypes.push_back(getResult().getType());
  for (Value root : roots)
    resultTypes.push_back(root.getType());

  AllocOp rebuilt = AllocOp::create(
      rewriter, getLoc(), resultTypes, roots, getProperties(),
      getOperation()->getDiscardableAttrDictionary().getValue());

  return cast<SafepointOpInterface>(rebuilt.getOperation());
}

// -----------------------------------------------------------------------------
// AllocaOp: PromotableMemOpInterface
// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// LoadOp: PromotableMemOpInterface
// -----------------------------------------------------------------------------

bool LoadOp::loadsFrom(const mlir::MemorySlot &slot) {
  return getAddress() == slot.ptr;
}

bool LoadOp::storesTo(const mlir::MemorySlot &slot) { return false; }

mlir::Value LoadOp::getStored(const mlir::MemorySlot &slot,
                              mlir::OpBuilder &builder,
                              mlir::Value reachingDefinition,
                              const mlir::DataLayout &dataLayout) {
  llvm_unreachable("getStored should not be called on LoadOp");
}

bool LoadOp::canUsesBeRemoved(
    const mlir::MemorySlot &slot,
    const llvm::SmallPtrSetImpl<mlir::OpOperand *> &blockingUses,
    llvm::SmallVectorImpl<mlir::OpOperand *> &newBlockingUses,
    const mlir::DataLayout &dataLayout) {
  if (blockingUses.size() != 1)
    return false;

  mlir::Value blockingUse = (*blockingUses.begin())->get();
  return blockingUse == slot.ptr && getAddress() == slot.ptr &&
         getResult().getType() == slot.elemType;
}

mlir::DeletionKind LoadOp::removeBlockingUses(
    const mlir::MemorySlot &slot,
    const llvm::SmallPtrSetImpl<mlir::OpOperand *> &blockingUses,
    mlir::OpBuilder &builder, mlir::Value reachingDefinition,
    const mlir::DataLayout &dataLayout) {
  getResult().replaceAllUsesWith(reachingDefinition);
  return mlir::DeletionKind::Delete;
}

// -----------------------------------------------------------------------------
// StoreOp: PromotableMemOpInterface
// -----------------------------------------------------------------------------

bool StoreOp::loadsFrom(const mlir::MemorySlot &slot) { return false; }

bool StoreOp::storesTo(const mlir::MemorySlot &slot) {
  return getAddress() == slot.ptr;
}

mlir::Value StoreOp::getStored(const mlir::MemorySlot &slot,
                               mlir::OpBuilder &builder,
                               mlir::Value reachingDefinition,
                               const mlir::DataLayout &dataLayout) {
  return getValue();
}

bool StoreOp::canUsesBeRemoved(
    const mlir::MemorySlot &slot,
    const llvm::SmallPtrSetImpl<mlir::OpOperand *> &blockingUses,
    llvm::SmallVectorImpl<mlir::OpOperand *> &newBlockingUses,
    const mlir::DataLayout &dataLayout) {
  if (blockingUses.size() != 1)
    return false;

  mlir::Value blockingUse = (*blockingUses.begin())->get();
  return blockingUse == slot.ptr && getAddress() == slot.ptr &&
         getValue() != slot.ptr && getValue().getType() == slot.elemType;
}

mlir::DeletionKind StoreOp::removeBlockingUses(
    const mlir::MemorySlot &slot,
    const llvm::SmallPtrSetImpl<mlir::OpOperand *> &blockingUses,
    mlir::OpBuilder &builder, mlir::Value reachingDefinition,
    const mlir::DataLayout &dataLayout) {
  return mlir::DeletionKind::Delete;
}

// -----------------------------------------------------------------------------
// PtrCastOp
// -----------------------------------------------------------------------------

mlir::LogicalResult PtrCastOp::verify() {
  mlir::Type sourceTy = getSource().getType();
  mlir::Type resultTy = getResult().getType();

  bool sourceIsDerived = isa<gc::DerivedPtrType>(sourceTy);
  bool resultIsDerived = isa<gc::DerivedPtrType>(resultTy);

  if (sourceIsDerived != resultIsDerived)
    return emitOpError("must preserve whether the pointer is derived");

  return success();
}

} // namespace gc
} // namespace mlir
