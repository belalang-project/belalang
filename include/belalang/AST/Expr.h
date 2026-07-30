#ifndef BELALANG_AST_EXPR_H_
#define BELALANG_AST_EXPR_H_

#include "belalang/AST/ASTContext.h"
#include "belalang/Lexer/Token.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/StringRef.h"

namespace belalang {
namespace ast {

class Stmt;
class VarDecl;

// -----------------------------------------------------------------------------
// Base Expression
// -----------------------------------------------------------------------------

class BlockExpr;

class Expr {
public:
  Expr() = delete;
  Expr(const Expr &) = delete;
  Expr(Expr &&) = delete;
  Expr &operator=(const Expr &) = delete;
  Expr &operator=(Expr &&) = delete;

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

  enum class ExprKind {
    BoolExpr,
    IntLitExpr,
    FloatLitExpr,
    StringLitExpr,
    ArrayLitExpr,
    VarExpr,
    FunctionLitExpr,
    CallExpr,
    IndexExpr,
    IdentifierExpr,
    IfExpr,
    InfixExpr,
    PrefixExpr,
    BlockExpr,
    MemberAccessExpr,
    StructLiteralExpr,
  };

  ExprKind getKind() const { return kind; }
  size_t getSpanStart() const { return spanStart; }
  size_t getSpanEnd() const { return spanEnd; }

protected:
  Expr(ExprKind kind, size_t spanStart, size_t spanEnd)
      : kind(kind), spanStart(spanStart), spanEnd(spanEnd) {};

private:
  ExprKind kind;
  size_t spanStart;
  size_t spanEnd;
};

// -----------------------------------------------------------------------------
// Derived Expressions
// -----------------------------------------------------------------------------

/// Represents boolean expressions, e.g. `true`
class BoolExpr : public Expr {
  bool value;

public:
  BoolExpr(size_t spanStart, size_t spanEnd, bool value)
      : Expr(ExprKind::BoolExpr, spanStart, spanEnd), value(value) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::BoolExpr;
  }

  bool getValue() const { return value; }
};

/// Represents integer literals, e.g. `42`
class IntLitExpr : public Expr {
  int64_t value;

public:
  IntLitExpr(size_t spanStart, size_t spanEnd, int64_t value)
      : Expr(ExprKind::IntLitExpr, spanStart, spanEnd), value(value) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::IntLitExpr;
  }

  int64_t getValue() const { return value; }
};

/// Represents floating point literals, e.g. `42.0`
class FloatLitExpr : public Expr {
  double value;

public:
  FloatLitExpr(size_t spanStart, size_t spanEnd, double value)
      : Expr(ExprKind::FloatLitExpr, spanStart, spanEnd), value(value) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::FloatLitExpr;
  }

  double getValue() const { return value; }
};

/// Represents string literals, e.g. `"hello"`
class StringLitExpr : public Expr {
  llvm::StringRef value;

public:
  StringLitExpr(size_t spanStart, size_t spanEnd, llvm::StringRef value)
      : Expr(ExprKind::StringLitExpr, spanStart, spanEnd), value(value) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::StringLitExpr;
  }

  llvm::StringRef getValue() const { return value; }
};

/// Represents array literals, e.g. `[1, 2, 3, "hello"]`
class ArrayLitExpr : public Expr {
  llvm::ArrayRef<Expr *> elements;

public:
  ArrayLitExpr(size_t spanStart, size_t spanEnd,
               llvm::ArrayRef<Expr *> elements)
      : Expr(ExprKind::ArrayLitExpr, spanStart, spanEnd), elements(elements) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::ArrayLitExpr;
  }

  llvm::ArrayRef<Expr *> getElements() const { return elements; }
};

/// Represents variable assignment expressions, e.g. `x = 12`
class VarExpr : public Expr {
  llvm::StringRef name;
  Expr *value;
  lexer::TokenKind assignOp;

public:
  VarExpr(size_t spanStart, size_t spanEnd, llvm::StringRef name, Expr *value,
          lexer::TokenKind assignOp)
      : Expr(ExprKind::VarExpr, spanStart, spanEnd), name(name), value(value),
        assignOp(assignOp) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::VarExpr;
  }

  llvm::StringRef getName() const { return name; }
  Expr *getValue() const { return value; }
  lexer::TokenKind getAssignOp() const { return assignOp; }
};

/// Represents function expression, e.g. `fn() { ... }`
class FunctionLitExpr : public Expr {
  llvm::ArrayRef<VarDecl *> params;
  BlockExpr *body;
  llvm::StringRef explicitType;

public:
  FunctionLitExpr(size_t spanStart, size_t spanEnd,
                  llvm::ArrayRef<VarDecl *> params, BlockExpr *body,
                  llvm::StringRef explicitType)
      : Expr(ExprKind::FunctionLitExpr, spanStart, spanEnd), params(params),
        body(body), explicitType(explicitType) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::FunctionLitExpr;
  }

  llvm::ArrayRef<VarDecl *> getParams() const { return params; }
  BlockExpr *getBody() const { return body; }
  llvm::StringRef getExplicitType() const { return explicitType; }
};

/// Represents call expression, e.g. `function_name()`
class CallExpr : public Expr {
  Expr *callee;
  llvm::ArrayRef<Expr *> args;

public:
  CallExpr(size_t spanStart, size_t spanEnd, Expr *callee,
           llvm::ArrayRef<Expr *> args)
      : Expr(ExprKind::CallExpr, spanStart, spanEnd), callee(callee),
        args(args) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::CallExpr;
  }

  Expr *getCallee() const { return callee; }
  llvm::ArrayRef<Expr *> getArgs() const { return args; }
};

/// Represents indexing expression, e.g. `a[1]`
class IndexExpr : public Expr {
  Expr *left;
  Expr *index;

public:
  IndexExpr(size_t spanStart, size_t spanEnd, Expr *left, Expr *index)
      : Expr(ExprKind::IndexExpr, spanStart, spanEnd), left(left),
        index(index) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::IndexExpr;
  }

  Expr *getLeft() const { return left; }
  Expr *getIndex() const { return index; }
};

/// Represents identifier expression, e.g. `variable_name`
class IdentifierExpr : public Expr {
  llvm::StringRef name;

public:
  IdentifierExpr(size_t spanStart, size_t spanEnd, llvm::StringRef name)
      : Expr(ExprKind::IdentifierExpr, spanStart, spanEnd), name(name) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::IdentifierExpr;
  }

  llvm::StringRef getName() const { return name; }
};

/// Represents if expressions, e.g. `if (...) { ... } else { ... }`
class IfExpr : public Expr {
  Expr *cond;
  BlockExpr *then;
  Expr *alt; // TODO: maybe pointer union?

public:
  IfExpr(size_t spanStart, size_t spanEnd, Expr *cond, BlockExpr *then,
         Expr *alt)
      : Expr(ExprKind::IfExpr, spanStart, spanEnd), cond(cond), then(then),
        alt(alt) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::IfExpr;
  }

  Expr *getCond() const { return cond; }
  BlockExpr *getThen() const { return then; }
  Expr *getAlt() const { return alt; }
};

/// Represents infix expression, e.g. `x + y`
class InfixExpr : public Expr {
  Expr *left;
  Expr *right;
  lexer::TokenKind infixOp;

public:
  InfixExpr(size_t spanStart, size_t spanEnd, Expr *left, Expr *right,
            lexer::TokenKind infixOp)
      : Expr(ExprKind::InfixExpr, spanStart, spanEnd), left(left), right(right),
        infixOp(infixOp) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::InfixExpr;
  }

  Expr *getLeft() const { return left; }
  Expr *getRight() const { return right; }
  lexer::TokenKind getInfixOp() const { return infixOp; }
};

/// Represents prefix expression, e.g. `-y`
class PrefixExpr : public Expr {
  lexer::TokenKind prefixOp;
  Expr *right;

public:
  PrefixExpr(size_t spanStart, size_t spanEnd, lexer::TokenKind prefixOp,
             Expr *right)
      : Expr(ExprKind::PrefixExpr, spanStart, spanEnd), prefixOp(prefixOp),
        right(right) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::PrefixExpr;
  }

  lexer::TokenKind getPrefixOp() const { return prefixOp; }
  Expr *getRight() const { return right; }
};

/// Represents block expression, e.g. `{ ... }`
class BlockExpr : public Expr {
  llvm::ArrayRef<Stmt *> stmts;

public:
  BlockExpr(size_t spanStart, size_t spanEnd, llvm::ArrayRef<Stmt *> stmts)
      : Expr(ExprKind::BlockExpr, spanStart, spanEnd), stmts(stmts) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::BlockExpr;
  }

  llvm::ArrayRef<Stmt *> getStmts() const { return stmts; }
};

/// Represents an access to member, e.g. `variable_name.member`
class MemberAccessExpr : public Expr {
  Expr *source;
  llvm::StringRef member;

public:
  MemberAccessExpr(size_t spanStart, size_t spanEnd, Expr *source,
                   llvm::StringRef member)
      : Expr(ExprKind::MemberAccessExpr, spanStart, spanEnd), source(source),
        member(member) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::MemberAccessExpr;
  }

  Expr *getSource() const { return source; }
  llvm::StringRef getMember() const { return member; }
};

/// Represents struct literal expression, e.g. `StructName { field = 12 }`
class StructLiteralExpr : public Expr {
  llvm::StringRef name;
  llvm::ArrayRef<VarExpr *> fields;

public:
  StructLiteralExpr(size_t spanStart, size_t spanEnd, llvm::StringRef name,
                    llvm::ArrayRef<VarExpr *> fields)
      : Expr(ExprKind::StructLiteralExpr, spanStart, spanEnd), name(name),
        fields(fields) {}

  static bool classof(const Expr *t) {
    return t->getKind() == ExprKind::StructLiteralExpr;
  }

  llvm::StringRef getName() const { return name; }
  llvm::ArrayRef<VarExpr *> getFields() const { return fields; }
};

} // namespace ast
} // namespace belalang

#endif // BELALANG_AST_EXPR_H_
