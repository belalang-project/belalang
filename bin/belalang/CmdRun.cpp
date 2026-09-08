#include "Cmds.h"
#include "Ctx.h"
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
    llvm::errs() << "error: could not open " << source << "\n";
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
    llvm::errs() << "error: BIR lowering pipeline failed\n";
    return 1;
  }

  auto tempDirectoryResult = createTemporaryDirectory("belalang-run");
  if (!tempDirectoryResult) {
    llvm::errs() << "error: " << tempDirectoryResult.takeError() << "\n";
    return 1;
  }
  Path tempDirectory = *tempDirectoryResult;

  std::string objFile = pathInDirectory(tempDirectory, "output.o").str().str();
  std::string
      exeFile = pathInDirectory(tempDirectory, "output.exe").str().str();

  llvmgen::LLVMGen llvmgen(birgen.getModulePtr());
  llvmgen.compileObjFile(objFile, llvmgen::SanitizerKind::None);

  auto linkResult = link(ctx, objFile, exeFile);
  if (!linkResult) {
    llvm::errs() << "error: " << linkResult.takeError() << "\n";
    removeTemporaryDirectory(tempDirectory);
    return 1;
  }
  if (*linkResult != 0) {
    llvm::errs() << "error: linking failed\n";
    removeTemporaryDirectory(tempDirectory);
    return 1;
  }

  auto executeResult = execute(exeFile);
  if (!executeResult) {
    llvm::errs() << "error: " << executeResult.takeError() << "\n";
    removeTemporaryDirectory(tempDirectory);
    return 1;
  }

  removeTemporaryDirectory(tempDirectory);
  return *executeResult;
}

} // namespace cmd
} // namespace belalang
