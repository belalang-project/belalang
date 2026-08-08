#ifndef BELALANG_AST_ASTDUMPER_H_
#define BELALANG_AST_ASTDUMPER_H_

#include "belalang/AST/ASTContext.h"
#include "belalang/AST/ASTVisitor.h"
#include "belalang/AST/Program.h"
#include "belalang/AST/Stmt.h"
#include "llvm/Support/raw_ostream.h"

namespace belalang {
namespace ast {

class ASTDumper : public ASTVisitor<ASTDumper> {
  ASTContext &ctx;
  llvm::raw_ostream &out;
  uint64_t depth;

  struct IndentGuard {
    uint64_t &depth;
    IndentGuard(uint64_t &depth) : depth(depth) { depth += 2; }
    ~IndentGuard() { depth -= 2; }
  };

public:
  ASTDumper(ASTContext &ctx, llvm::raw_ostream &out = llvm::outs())
      : ctx(ctx), out(out), depth(0) {}

  void visitProgram(Program *prog);

#define STMT(name) void visit##name##Stmt(name##Stmt *stmt);
#define EXPR(name) void visit##name##Expr(name##Expr *expr);
#define DECL(name) void visit##name##Decl(name##Decl *decl);
#include "belalang/AST/ASTNodes.def"
};

} // namespace ast
} // namespace belalang

#endif // BELALANG_AST_ASTDUMPER_H_
