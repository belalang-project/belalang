#ifndef BELALANG_AST_DECL_H_
#define BELALANG_AST_DECL_H_

#include "belalang/AST/ASTContext.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

namespace belalang {
namespace ast {

class Expr;

// -----------------------------------------------------------------------------
// Base Decl
// -----------------------------------------------------------------------------

class Decl {
public:
  Decl() = delete;
  Decl(const Decl &) = delete;
  Decl(Decl &&) = delete;
  Decl &operator=(const Decl &) = delete;
  Decl &operator=(Decl &&) = delete;

  void *operator new(size_t bytes, const ASTContext &ctx,
                     unsigned alignment = 8) {
    return ::operator new(bytes, ctx, alignment);
  }
  void *operator new(size_t bytes, const ASTContext *ctx,
                     unsigned alignment = 8) {
    return ::operator new(bytes, *ctx, alignment);
  }

  void *operator new(size_t bytes) = delete;
  void operator delete(void *ptr) = delete;

  enum class DeclKind {
    VarDecl,
    StructDecl,
  };

  DeclKind getKind() const { return kind; }
  size_t getSpanStart() const { return spanStart; }
  size_t getSpanEnd() const { return spanEnd; }

protected:
  Decl(DeclKind kind, size_t spanStart, size_t spanEnd)
      : kind(kind), spanStart(spanStart), spanEnd(spanEnd) {}

private:
  DeclKind kind;
  size_t spanStart;
  size_t spanEnd;
};

// -----------------------------------------------------------------------------
// Derived Decls
// -----------------------------------------------------------------------------

/// Represents variable declaration, e.g. `x := 1`
class VarDecl : public Decl {
  llvm::StringRef name;
  Expr *value;
  llvm::StringRef explicitType;

public:
  VarDecl(size_t spanStart, size_t spanEnd, llvm::StringRef name, Expr *value,
          llvm::StringRef explicitType)
      : Decl(DeclKind::VarDecl, spanStart, spanEnd), name(name), value(value),
        explicitType(explicitType) {}

  static bool classof(const Decl *t) {
    return t->getKind() == DeclKind::VarDecl;
  }

  llvm::StringRef getName() const { return name; }
  Expr *getValue() const { return value; }
  llvm::StringRef getExplicitType() const { return explicitType; }
};

/// Represents struct declaration, e.g. `struct T { ... }`
class StructDecl : public Decl {
  llvm::StringRef name;
  llvm::ArrayRef<VarDecl *> fields;

public:
  StructDecl(size_t spanStart, size_t spanEnd, llvm::StringRef name,
             llvm::ArrayRef<VarDecl *> fields)
      : Decl(DeclKind::StructDecl, spanStart, spanEnd), name(name),
        fields(fields) {}

  static bool classof(const Decl *t) {
    return t->getKind() == DeclKind::StructDecl;
  }

  llvm::StringRef getName() const { return name; }
  llvm::ArrayRef<VarDecl *> getFields() const { return fields; }
};

} // namespace ast
} // namespace belalang

#endif // BELALANG_AST_DECL_H_
