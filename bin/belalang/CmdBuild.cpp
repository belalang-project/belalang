#include "belalang/AST/Parser.h"
#include "belalang/AST/ASTDumper.h"
#include "belalang/BIRGen/BIRGen.h"
#include "belalang/Lexer/Lexer.h"
#include "belalang/Diag/Diag.h"
#include "llvm/Support/MemoryBuffer.h"
#include <iostream>
#include <string>

#include "Cmds.h"

// A very simple implementation. There maybe another more elegant way, but I
// haven't figured it out. Maybe also move this to a header file under lexer.
static std::string escapeString(llvm::StringRef str) {
  std::string result;
  for (char c : str) {
    switch (c) {
    case '\n':
      result += "\\n";
      break;
    case '\r':
      result += "\\r";
      break;
    case '\t':
      result += "\\t";
      break;
    case '"':
      result += "\\\"";
      break;
    case '\\':
      result += "\\\\";
      break;
    default:
      result += c;
      break;
    }
  }
  return result;
}

namespace belalang {
namespace cmd {

enum class EmitTarget {
  Bir,
  Ast,
  Tokens,
  Llvm,
  Obj,
  Exe,
};

int build(muopt::Parser &parser) {
  auto emit = EmitTarget::Exe;
  std::string source;

  while (auto arg = parser.next()) {
    if (arg.has_value() && arg->is_long("emit")) {
      auto value = parser.arg_value();
      if (value == "tokens")
        emit = EmitTarget::Tokens;
      if (value == "ast")
        emit = EmitTarget::Ast;
      if (value == "bir")
        emit = EmitTarget::Bir;
    }
    if (arg.has_value() && arg->is_plain()) {
      source = arg->as_str();
    }
  }

  auto fileBuf = llvm::MemoryBuffer::getFileOrSTDIN(source);
  if (!fileBuf) {
    std::cerr << "error: could not open " << source << "\n";
    return 1;
  }
  llvm::StringRef src = (*fileBuf)->getBuffer();

  diag::DiagnosticEngine diagEngine(src, source, false);

  if (emit == EmitTarget::Tokens) {
    lexer::Lexer lexer(src, diagEngine);
    lexer::Token tok;
    bool hasError = false;

    while (true) {
      if (!lexer.lex(tok)) {
        if (tok.getKind() != lexer::TokenKind::EndOfFile) {
          hasError = true;
        }
        break;
      }
      auto [start, end] = tok.getSpan();
      auto kindStr = tok.getString();
      auto kind = tok.getKind();
      if (kind == lexer::TokenKind::LiteralString) {
        std::cout << kindStr << " \"" << escapeString(tok.getStr()) << "\" <"
                  << start << ".." << end << ">\n";
      } else if (kind == lexer::TokenKind::Ident ||
                 kind == lexer::TokenKind::LiteralInteger ||
                 kind == lexer::TokenKind::LiteralFloat) {
        std::cout << kindStr << " \"" << tok.getStr().str() << "\" <"
                  << start << ".." << end << ">\n";
      } else {
        std::cout << kindStr << " <" << start << ".." << end << ">\n";
      }
    }

    return hasError ? 1 : 0;
  }

  if (emit == EmitTarget::Ast) {
    lexer::Lexer lexer(src, diagEngine);

    ast::ASTContext astCtx;
    ast::Parser parser(lexer, astCtx, diagEngine);

    ast::Program *prog = parser.parseProgram();

    if (prog) {
      ast::ASTDumper dumper;
      dumper.visitProgram(prog);
    }

    return parser.hadError() ? 1 : 0;
  }

  if (emit == EmitTarget::Bir) {
    lexer::Lexer lexer(src, diagEngine);

    ast::ASTContext astCtx;
    ast::Parser parser(lexer, astCtx, diagEngine);

    ast::Program *prog = parser.parseProgram();
    if (!prog)
      return parser.hadError() ? 1 : 0;

    birgen::BIRGen birgen(diagEngine);
    birgen.generateProgram(prog);

    if (!birgen.runLoweringPipeline()) {
      std::cerr << "error: BIR lowering pipeline failed\n";
      return 1;
    }

    std::cout << birgen.dumpToString() << "\n";
    return parser.hadError() ? 1 : 0;
  }

  std::cout << "error: unimplemented\n";
  return 1;
}

} // namespace cmd
} // namespace belalang
