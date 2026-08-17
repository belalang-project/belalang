#ifndef BELALANG_BIR_TRANSFORMS_OPTIMIZE_STRUCT_LAYOUT_H_
#define BELALANG_BIR_TRANSFORMS_OPTIMIZE_STRUCT_LAYOUT_H_

#include "belalang/BIR/IR/BIR.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"

namespace belalang {
namespace bir {

struct StructMemberLayout {
  uint64_t alignment;
  uint64_t size;

  StructMemberLayout(uint64_t alignment, uint64_t size)
      : alignment(alignment), size(size) {}
};

llvm::SmallVector<int32_t>
computeStructReorder(llvm::ArrayRef<StructMemberLayout> members);

mlir::LogicalResult optimizeStructLayout(const mlir::DataLayout &dl,
                                         StructType type);

} // namespace bir
} // namespace belalang

#endif // BELALANG_BIR_TRANSFORMS_OPTIMIZE_STRUCT_LAYOUT_H_
