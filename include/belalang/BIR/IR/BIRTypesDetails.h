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

    KeyTy(llvm::ArrayRef<mlir::Type> members, mlir::StringAttr name)
        : members(members), name(name) {}
  };

  StructTypeStorage(llvm::ArrayRef<mlir::Type> members, mlir::StringAttr name)
      : members(members), name(name) {}

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
    return new (allocator.allocate<StructTypeStorage>())
        StructTypeStorage(members, key.name);
  }

  mlir::LogicalResult mutate(mlir::TypeStorageAllocator &allocator,
                             llvm::ArrayRef<mlir::Type> members,
                             mlir::StringAttr name) {
    if (this->members == members)
      return mlir::success();

    // Already defined differently.
    if (!this->members.empty())
      return mlir::failure();

    this->members = allocator.copyInto(members);
    this->name = name;
    return mlir::success();
  }

  llvm::ArrayRef<mlir::Type> members;
  mlir::StringAttr name;
};

} // namespace detail
} // namespace bir
} // namespace belalang

#endif // BELALANG_BIR_IR_BIRTYPESDETAILS_H_
