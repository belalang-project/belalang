#ifndef BELALANG_BIR_IR_BIRTYPESDETAILS_H_
#define BELALANG_BIR_IR_BIRTYPESDETAILS_H_

#include "mlir/IR/BuiltinAttributes.h"
#include "mlir/IR/TypeSupport.h"

namespace belalang {
namespace bir {
namespace detail {

struct StructTypeStorage : public mlir::TypeStorage {
public:
  struct KeyTy {
    llvm::ArrayRef<mlir::Type> members;
    mlir::StringAttr name;
    llvm::ArrayRef<int32_t> reorder;

    KeyTy(llvm::ArrayRef<mlir::Type> members, mlir::StringAttr name,
          llvm::ArrayRef<int32_t> reorder)
        : members(members), name(name), reorder(reorder) {}
  };

  StructTypeStorage(llvm::ArrayRef<mlir::Type> members, mlir::StringAttr name,
                    llvm::ArrayRef<int32_t> reorder)
      : members(members), name(name), reorder(reorder) {}

  // ---------------------------------------------------------------------------
  // Type Uniquing Infrastructure
  // ---------------------------------------------------------------------------

  bool operator==(const KeyTy &key) const { return key.name == name; }

  static llvm::hash_code hashKey(const KeyTy &key) {
    return llvm::hash_value(key.name);
  }

  static StructTypeStorage *construct(mlir::TypeStorageAllocator &allocator,
                                      const KeyTy &key) {
    llvm::ArrayRef<mlir::Type> members = allocator.copyInto(key.members);
    llvm::ArrayRef<int32_t> reorder = allocator.copyInto(key.reorder);
    return new (allocator.allocate<StructTypeStorage>())
        StructTypeStorage(members, key.name, reorder);
  }

  mlir::LogicalResult mutate(mlir::TypeStorageAllocator &allocator,
                             llvm::ArrayRef<mlir::Type> members,
                             mlir::StringAttr name,
                             llvm::ArrayRef<int32_t> reorder) {
    if (this->members == members && this->reorder == reorder)
      return mlir::success();

    // Already defined differently.
    if (!this->members.empty() && this->members != members)
      return mlir::failure();

    this->members = allocator.copyInto(members);
    this->name = name;
    this->reorder = allocator.copyInto(reorder);
    return mlir::success();
  }

  llvm::ArrayRef<mlir::Type> members;
  mlir::StringAttr name;
  llvm::ArrayRef<int32_t> reorder;
};

} // namespace detail
} // namespace bir
} // namespace belalang

#endif // BELALANG_BIR_IR_BIRTYPESDETAILS_H_
