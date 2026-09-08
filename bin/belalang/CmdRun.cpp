#include "Cmds.h"
#include "Ctx.h"
#include "belalang/AST/Parser.h"
#include "belalang/BIRGen/BIRGen.h"
#include "belalang/Diag/Diag.h"
#include "belalang/LLVMGen/LLVMGen.h"
#include "belalang/Lexer/Lexer.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include <iostream>
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
    std::cerr << "error: could not open " << source << "\n";
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
    std::cerr << "error: BIR lowering pipeline failed\n";
    return 1;
  }

  llvmgen::LLVMGen llvmgen(birgen.getModulePtr());
  llvm::SmallString<128> tempDir;
  if (auto ec = llvm::sys::fs::createUniqueDirectory("belalang-out", tempDir)) {
    std::cerr << "error: " << ec.message() << "\n";
    return 1;
  }

  llvm::SmallString<128> objPath(tempDir);
  llvm::sys::path::append(objPath, "output.o");
  std::string objFile = objPath.str().str();

  llvmgen.compileObjFile(objFile, llvmgen::SanitizerKind::None);

  llvm::SmallString<128> exePath(tempDir);
  llvm::sys::path::append(exePath, "output.exe");
  std::string exeFile = exePath.str().str();

  std::string libraryPath = "-L" + ctx.brt_dir;
  std::string stackmapsArg =
      "-Wl,-T," + ctx.brt_dir + "/llvm_stackmaps.ld";
  llvm::SmallVector<llvm::StringRef, 8> linkArgs = {
      ctx.cc_cmd,
      "-no-pie",
      objFile,
      libraryPath,
      stackmapsArg,
      "-lbrt",
      "-o",
      exeFile,
  };

  if (llvm::sys::ExecuteAndWait(ctx.cc_cmd, linkArgs) != 0) {
    std::cerr << "error: linking failed\n";
    if (auto ec = llvm::sys::fs::remove_directories(tempDir)) {
      std::cerr << "error: " << ec.message() << "\n";
    };
    return 1;
  }

  llvm::SmallVector<llvm::StringRef, 1> runArgs = {exeFile};
  int res = llvm::sys::ExecuteAndWait(exeFile, runArgs);
  if (auto ec = llvm::sys::fs::remove_directories(tempDir)) {
    std::cerr << "error: " << ec.message() << "\n";
  };

  return res;
}

} // namespace cmd
} // namespace belalang
