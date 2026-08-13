#include "belalang/AST/Parser.h"
#include "belalang/BIRGen/BIRGen.h"
#include "belalang/LLVMGen/LLVMGen.h"
#include "belalang/Lexer/Lexer.h"
#include "belalang/Diag/Diag.h"
#include "llvm/Support/MemoryBuffer.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>
#include "Cmds.h"

namespace belalang {
namespace cmd {

int run(muopt::Parser &parser) {
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
  
  const char *brt_dir = std::getenv("BRT_DIR");
  std::string brt_path = brt_dir ? brt_dir : "/usr/local/lib";

  const char *cc = std::getenv("CC");
  std::string cc_cmd = cc ? cc : "cc";

  std::string linkCmd = cc_cmd + " -no-pie " + objFile + " -L" + brt_path +
                        " -Wl,-T," + brt_path +
                        "/llvm_stackmaps.ld"
                        " -lbrt -o " +
                        exeFile;
  if (std::system(linkCmd.c_str()) != 0) {
    std::cerr << "error: linking failed\n";
    std::remove(objFile.c_str());
    return 1;
  }

  int res = std::system(exeFile.c_str());
  
  std::remove(objFile.c_str());
  std::remove(exeFile.c_str());

  if (WIFEXITED(res)) {
    return WEXITSTATUS(res);
  }

  return 1;
}

} // namespace cmd
} // namespace belalang
