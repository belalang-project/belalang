#include "Cmds.h"
#include "Ctx.h"
#include "belalang/AST/Parser.h"
#include "belalang/BIRGen/BIRGen.h"
#include "belalang/Diag/Diag.h"
#include "belalang/LLVMGen/LLVMGen.h"
#include "belalang/Lexer/Lexer.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Program.h"
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

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
  std::string objFile = "/tmp/belalang_out_" + std::to_string(getpid()) + ".o";
  llvmgen.compileObjFile(objFile, llvmgen::SanitizerKind::None);

  std::string exeFile = "/tmp/belalang_exe_" + std::to_string(getpid());

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
    std::remove(objFile.c_str());
    return 1;
  }

  llvm::SmallVector<llvm::StringRef, 1> runArgs = {exeFile};
  int res = llvm::sys::ExecuteAndWait(exeFile, runArgs);

  std::remove(objFile.c_str());
  std::remove(exeFile.c_str());
  return res;
}

} // namespace cmd
} // namespace belalang
