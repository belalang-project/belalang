#ifndef BELALANG_AST_PROGRAM_H_
#define BELALANG_AST_PROGRAM_H_

#include "belalang/AST/ASTContext.h"
#include "belalang/AST/Stmt.h"
#include "llvm/ADT/ArrayRef.h"

namespace belalang {
namespace ast {

class Program {
  llvm::ArrayRef<Stmt *> stmts;

public:
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

  Program(llvm::ArrayRef<Stmt *> stmts) : stmts(stmts) {}

  llvm::ArrayRef<Stmt *> getStmts() const { return stmts; }
};

} // namespace ast
} // namespace belalang

#endif // BELALANG_AST_PROGRAM_H_
