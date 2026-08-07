#ifndef BELALANG_AST_ASTCONTEXT_H_
#define BELALANG_AST_ASTCONTEXT_H_

#include "belalang/AST/Type.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Allocator.h"

namespace belalang {
namespace ast {

class ASTContext {
  mutable llvm::BumpPtrAllocator allocator;

  // ---------------------------------------------------------------------------
  // Types
  // ---------------------------------------------------------------------------

  mutable llvm::SmallVector<Type *, 0> types;

public:
  ASTContext();

  // ---------------------------------------------------------------------------
  // Allocation goodies
  // ---------------------------------------------------------------------------

  llvm::BumpPtrAllocator &getAllocator() const { return allocator; }

  void *alloc(size_t size, unsigned align = 8) const {
    return allocator.Allocate(size, align);
  }
  template <typename T> T *alloc(size_t num = 1) const {
    return static_cast<T *>(alloc(num * sizeof(T), alignof(T)));
  }
  void dealloc(void *ptr) const {}

  // ---------------------------------------------------------------------------
  // Builtin Types
  // ---------------------------------------------------------------------------

  void initBuiltinTypes();
  void initBuiltinType(BuiltinType *&ty, BuiltinType::Kind kind);

  BuiltinType *intTy;
  BuiltinType *floatTy;
  BuiltinType *boolTy;
  BuiltinType *stringTy;
  BuiltinType *noneTy;

  // ---------------------------------------------------------------------------
  // Type name conversions
  // ---------------------------------------------------------------------------

  llvm::StringRef tyToStr(Type *ty) const;
  Type *strToTy(llvm::StringRef str) const;
};

} // namespace ast
} // namespace belalang

inline void *operator new(size_t bytes, const belalang::ast::ASTContext &ctx,
                          size_t alignment = 8) {
  return ctx.alloc(bytes, alignment);
}

inline void operator delete(void *ptr, const belalang::ast::ASTContext &ctx,
                            size_t) {
  return ctx.dealloc(ptr);
}

inline void *operator new[](size_t bytes, const belalang::ast::ASTContext &ctx,
                            size_t alignment = 8) {
  return ctx.alloc(bytes, alignment);
}

inline void operator delete[](void *ptr, const belalang::ast::ASTContext &ctx,
                              size_t) {
  return ctx.dealloc(ptr);
}

#endif // BELALANG_AST_ASTCONTEXT_H_
