#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"

namespace mlir {
namespace gc {

// -----------------------------------------------------------------------------
// PtrType
// -----------------------------------------------------------------------------

llvm::TypeSize PtrType::getTypeSizeInBits(const DataLayout &dataLayout,
                                          DataLayoutEntryListRef params) const {
  return dataLayout.getTypeSizeInBits(LLVM::LLVMPointerType::get(getContext()));
}

uint64_t PtrType::getABIAlignment(const DataLayout &dataLayout,
                                  DataLayoutEntryListRef params) const {
  return dataLayout.getTypeABIAlignment(
      LLVM::LLVMPointerType::get(getContext()));
}

// -----------------------------------------------------------------------------
// DerivedPtrType
// -----------------------------------------------------------------------------

llvm::TypeSize
DerivedPtrType::getTypeSizeInBits(const DataLayout &dataLayout,
                                  DataLayoutEntryListRef params) const {
  return dataLayout.getTypeSizeInBits(LLVM::LLVMPointerType::get(getContext()));
}

uint64_t DerivedPtrType::getABIAlignment(const DataLayout &dataLayout,
                                         DataLayoutEntryListRef params) const {
  return dataLayout.getTypeABIAlignment(
      LLVM::LLVMPointerType::get(getContext()));
}

// -----------------------------------------------------------------------------
// Utilities
// -----------------------------------------------------------------------------

Type getGCPointerPointee(Type type) {
  return llvm::TypeSwitch<Type, Type>(type)
      .Case<PtrType, DerivedPtrType>(
          [](auto pointer) { return pointer.getPointee(); })
      .Default(Type{});
}

} // namespace gc
} // namespace mlir
