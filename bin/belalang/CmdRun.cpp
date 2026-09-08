#include "Belalang.h"
#include "belalang/AST/Parser.h"
#include "belalang/BIRGen/BIRGen.h"
#include "belalang/Diag/Diag.h"
#include "belalang/LLVMGen/LLVMGen.h"
#include "belalang/Lexer/Lexer.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

namespace belalang {
namespace cmd {

int run(muopt::Parser &parser, const BelalangCtx &ctx) {
  std::string source;
  while (auto arg = parser.next()) {
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
  lexer::Lexer lexer(src, diagEngine);
  ast::ASTContext astCtx;
  ast::Parser astParser(lexer, astCtx, diagEngine);

  ast::Program *prog = astParser.parseProgram();
  if (!prog)
    return astParser.hadError() ? 1 : 0;

  birgen::BIRGen birgen(astCtx, diagEngine);
  birgen.generateProgram(prog);

  if (!birgen.runLoweringPipeline()) {
    term::error() << "BIR lowering pipeline failed\n";
    return 1;
  }

  auto tempDirRes = makeTempDir("belalang-run");
  if (!tempDirRes) {
    term::error() << tempDirRes.takeError() << "\n";
    return 1;
  }
  Path tempDir = *tempDirRes;

  Path objFile = pathIn(tempDir, "output.o");
  Path exeFile = pathIn(tempDir, "output.exe");

  llvmgen::LLVMGen llvmgen(birgen.getModulePtr());
  llvmgen.compileObjFile(objFile, llvmgen::SanitizerKind::None);

  auto linkResult = link(ctx, objFile, exeFile);
  if (!linkResult) {
    term::error() << linkResult.takeError() << "\n";
    removeTempDir(tempDir);
    return 1;
  }
  if (*linkResult != 0) {
    term::error() << "linking failed\n";
    removeTempDir(tempDir);
    return 1;
  }

  auto executeResult = execute(exeFile);
  if (!executeResult) {
    term::error() << executeResult.takeError() << "\n";
    removeTempDir(tempDir);
    return 1;
  }

  removeTempDir(tempDir);
  return *executeResult;
}

} // namespace cmd
} // namespace belalang
