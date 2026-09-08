#include "belalang/AST/ASTDumper.h"
#include "belalang/AST/Parser.h"
#include "belalang/BIR/Transforms/Passes.h"
#include "belalang/BIRGen/BIRGen.h"
#include "belalang/Diag/Diag.h"
#include "belalang/LLVMGen/LLVMGen.h"
#include "belalang/Lexer/Lexer.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <string_view>

#include "Belalang.h"

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

static bool parseBoolean(std::string_view value, bool &result) {
  if (value == "true") {
    result = true;
    return true;
  }
  if (value == "false") {
    result = false;
    return true;
  }
  return false;
}

static bool parseBIRGenOption(std::string_view option,
                              belalang::bir::BIRLoweringPipelineOptions &opts) {
  size_t eq = option.find('=');
  if (eq == std::string_view::npos) {
    belalang::cmd::term::error()
        << "malformed BIRGen option: " << option << "\n";
    belalang::cmd::term::hint() << "expected <name>=<true|false>\n";
    return false;
  }

  std::string_view name = option.substr(0, eq);
  std::string_view rawValue = option.substr(eq + 1);

  if (name == "enable-dce") {
    if (!parseBoolean(rawValue, opts.enableDCE.getValue())) {
      belalang::cmd::term::error()
          << "invalid value for BIRGen option '" << name << "': " << rawValue
          << "\n";
      belalang::cmd::term::hint() << "expected true or false\n";
      return false;
    }
    return true;
  }

  if (name == "enable-mem2reg") {
    if (!parseBoolean(rawValue, opts.enableMem2Reg.getValue())) {
      belalang::cmd::term::error()
          << "invalid value for BIRGen option '" << name << "': " << rawValue
          << "\n";
      belalang::cmd::term::hint() << "expected true or false\n";
      return false;
    }
    return true;
  }

  belalang::cmd::term::error() << "unknown BIRGen option: " << name << "\n";
  return false;
}

namespace belalang {
namespace cmd {

enum class EmitTarget {
  Bir,
  BirLowered,
  Ast,
  Tokens,
  Llvm,
  Obj,
  Exe,
};

int build(muopt::Parser &parser, const BelalangCtx &ctx) {
  auto emit = EmitTarget::Exe;
  bir::BIRLoweringPipelineOptions birOptions;
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
      if (value == "bir-lowered")
        emit = EmitTarget::BirLowered;
      if (value == "llvm")
        emit = EmitTarget::Llvm;
      if (value == "exe")
        emit = EmitTarget::Exe;
    }
    if (arg.has_value() && arg->is_long("bir-lowering-pipeline")) {
      auto value = parser.arg_value();
      if (!value.has_value()) {
        term::error() << "missing value for --birgen\n";
        return 1;
      }
      if (!parseBIRGenOption(*value, birOptions))
        return 1;
    }
    if (arg.has_value() && arg->is_plain()) {
      source = arg->as_str();
    }
  }

  auto fileBuf = llvm::MemoryBuffer::getFileOrSTDIN(source);
  if (!fileBuf) {
    term::error() << "could not open " << source << "\n";
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
        llvm::outs() << kindStr << " \"" << escapeString(tok.getStr()) << "\" <"
                     << start << ".." << end << ">\n";
      } else if (kind == lexer::TokenKind::Ident ||
                 kind == lexer::TokenKind::LiteralInteger ||
                 kind == lexer::TokenKind::LiteralFloat) {
        llvm::outs() << kindStr << " \"" << tok.getStr() << "\" <" << start
                     << ".." << end << ">\n";
      } else {
        llvm::outs() << kindStr << " <" << start << ".." << end << ">\n";
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
      ast::ASTDumper dumper(astCtx);
      dumper.visitProgram(prog);
    }

    return parser.hadError() ? 1 : 0;
  }

  if (emit == EmitTarget::Bir || emit == EmitTarget::BirLowered) {
    lexer::Lexer lexer(src, diagEngine);

    ast::ASTContext astCtx;
    ast::Parser parser(lexer, astCtx, diagEngine);

    ast::Program *prog = parser.parseProgram();
    if (!prog)
      return parser.hadError() ? 1 : 0;

    birgen::BIRGen birgen(astCtx, diagEngine);
    birgen.generateProgram(prog);

    if (emit == EmitTarget::BirLowered &&
        !birgen.runLoweringPipeline(birOptions)) {
      term::error() << "BIR lowering pipeline failed\n";
      return 1;
    }

    llvm::outs() << birgen.dumpToString() << "\n";
    return parser.hadError() ? 1 : 0;
  }

  if (emit == EmitTarget::Llvm) {
    lexer::Lexer lexer(src, diagEngine);

    ast::ASTContext astCtx;
    ast::Parser parser(lexer, astCtx, diagEngine);

    ast::Program *prog = parser.parseProgram();
    if (!prog)
      return parser.hadError() ? 1 : 0;

    birgen::BIRGen birgen(astCtx, diagEngine);
    birgen.generateProgram(prog);

    if (!birgen.runLoweringPipeline(birOptions)) {
      term::error() << "BIR lowering pipeline failed\n";
      return 1;
    }

    llvmgen::LLVMGen llvmgen(birgen.getModulePtr());
    llvm::outs() << llvmgen.dumpToString() << "\n";
    return 0;
  }

  if (emit == EmitTarget::Exe) {
    lexer::Lexer lexer(src, diagEngine);

    ast::ASTContext astCtx;
    ast::Parser parser(lexer, astCtx, diagEngine);

    ast::Program *prog = parser.parseProgram();
    if (!prog)
      return parser.hadError() ? 1 : 0;

    birgen::BIRGen birgen(astCtx, diagEngine);
    birgen.generateProgram(prog);

    if (!birgen.runLoweringPipeline(birOptions)) {
      term::error() << "BIR lowering pipeline failed\n";
      return 1;
    }

    auto tempDirRes = makeTempDir("belalang-build");
    if (!tempDirRes) {
      term::error() << tempDirRes.takeError() << "\n";
      return 1;
    }
    Path tempDir = *tempDirRes;

    std::string objFile = pathIn(tempDir, "output.o").str().str();
    std::string exeFile = executablePathFor(source).str().str();

    llvmgen::LLVMGen llvmgen(birgen.getModulePtr());
    llvmgen.compileObjFile(objFile, llvmgen::SanitizerKind::None);

    auto linkRes = link(ctx, objFile, exeFile);
    if (!linkRes) {
      term::error() << linkRes.takeError() << "\n";
      removeTempDir(tempDir);
      return 1;
    }
    if (*linkRes != 0) {
      term::error() << "linking failed\n";
      removeTempDir(tempDir);
      return 1;
    }

    removeTempDir(tempDir);

    return 0;
  }

  term::error() << "unimplemented\n";
  return 1;
}

} // namespace cmd
} // namespace belalang
