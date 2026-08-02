#ifndef BELALANG_AST_PARSER_H_
#define BELALANG_AST_PARSER_H_

#include "belalang/AST/Decl.h"
#include "belalang/AST/Expr.h"
#include "belalang/AST/Program.h"
#include "belalang/AST/Stmt.h"
#include "belalang/Diag/Diag.h"
#include "belalang/Lexer/Lexer.h"

namespace belalang {
namespace ast {

class Parser {
  // ---------------------------------------------------------------------------
  // Precedence
  // ---------------------------------------------------------------------------

  enum class Precedence : uint8_t {
    Lowest,
    AssignmentOps,
    LogicalOr,
    LogicalAnd,
    BitOr,
    BitXor,
    BitAnd,
    Equality,
    Relational,
    Shift,
    Additive,
    Multiplicative,
    Prefix,
    Call,
    Index,
  };

  static Precedence precedenceOf(lexer::TokenKind kind);

  // ---------------------------------------------------------------------------
  // Parser Restrictions
  // ---------------------------------------------------------------------------

  /// Parser restrictions options. This limits what the parser can parse in
  /// order to help with ambiguity during parsing.
  enum class Restr : uint8_t {
    None = 0,
    NoStructLiteral = 1 << 0,
  };

  /// RAII guard for parser restrictions.
  struct RestrGuard {
    Parser &p;
    Restr old;

    RestrGuard(Parser &p, Restr r) : p(p), old(p.restr) { p.restr = r; }
    ~RestrGuard() { p.restr = old; }
  };

  bool hasRestr(Restr has) {
    return static_cast<uint8_t>(restr) & static_cast<uint8_t>(has);
  }

  [[nodiscard]] RestrGuard restrict(Restr r) { return RestrGuard(*this, r); }

  // ---------------------------------------------------------------------------
  // Token Handling
  // ---------------------------------------------------------------------------

  /// Consumes token making `tok` the next un-"parsed" token.
  void consumeToken();

  /// Consumes token if the current token kind is expected.
  bool tryConsumeToken(lexer::TokenKind expectedTok);

  // ---------------------------------------------------------------------------
  // Diagnostics
  // ---------------------------------------------------------------------------

  /// Emits an error diagnostic at the given span, using `label` as the
  /// primary label message. Marks the parser as having failed.
  void errorAt(size_t spanStart, size_t spanEnd, std::string message,
               std::string label);

  /// Emits an error diagnostic at the given span using `message` as both the
  /// diagnostic and primary label message.
  void errorAt(size_t spanStart, size_t spanEnd, std::string message);

  // ---------------------------------------------------------------------------
  // Parse Methods
  // ---------------------------------------------------------------------------

  Stmt *parseStmt();
  Expr *parseExpr(Precedence prec);
  Expr *parsePrefix();
  Expr *parseInfix(Expr *left);
  Expr *parseGroupedExpr();
  Decl *parseDecl();

#define STMT(name) Stmt *parse##name##Stmt();
#define DECL(name) Decl *parse##name##Decl();
#include "ASTNodes.def"

  Expr *parseBoolExpr();
  Expr *parseIntLitExpr();
  Expr *parseFloatLitExpr();
  Expr *parseStringLitExpr();
  Expr *parseArrayLitExpr();
  Expr *parseFunctionLitExpr();
  Expr *parseIdentifierExpr();
  Expr *parseIfExpr();
  Expr *parsePrefixExpr();
  Expr *parseBlockExpr();
  Expr *parseInfixExpr(Expr *left);
  Expr *parseCallExpr(Expr *left);
  Expr *parseIndexExpr(Expr *left);
  Expr *parseVarExpr(Expr *left);
  Expr *parseMemberAccessExpr(Expr *left);
  Expr *parseStructLiteralExpr(Expr *left);

public:
  Parser(lexer::Lexer &lexer, ASTContext &c, diag::DiagnosticEngine &diagEngine)
      : lexer(lexer), c(c), diagEngine(diagEngine), restr(Restr::None) {
    lexer.lex(tok);
    lexer.lex(peekTok);
  }

  Program *parseProgram();

  /// Returns `true` if the parser encountered any error while parsing.
  bool hadError() const { return hasError; }

private:
  lexer::Lexer &lexer;
  ASTContext &c;
  diag::DiagnosticEngine &diagEngine;

  /// This token points to the next token waiting to be parsed.
  lexer::Token tok;

  /// This token points to the token after `tok`, used for lookahead.
  lexer::Token peekTok;

  int depth = 0;
  bool hasSemicolon = true;

  bool hasError = false;

  Restr restr;
};

} // namespace ast
} // namespace belalang

#endif // BELALANG_AST_PARSER_H_
