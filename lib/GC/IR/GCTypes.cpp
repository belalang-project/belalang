#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"

namespace mlir {
namespace gc {

llvm::TypeSize PtrType::getTypeSizeInBits(const DataLayout &dataLayout,
                                          DataLayoutEntryListRef params) const {
  return dataLayout.getTypeSizeInBits(LLVM::LLVMPointerType::get(getContext()));
}

uint64_t PtrType::getABIAlignment(const DataLayout &dataLayout,
                                  DataLayoutEntryListRef params) const {
  return dataLayout.getTypeABIAlignment(
      LLVM::LLVMPointerType::get(getContext()));
}

} // namespace gc
} // namespace mlir
