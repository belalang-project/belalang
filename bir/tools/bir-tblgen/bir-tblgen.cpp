#include "TableGenBackends.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Main.h"

using namespace belalang;

enum ActionType {
  GenRustBindingDecls,
  GenCXXBindingDecls,
  GenCXXBindingDefs,
};

llvm::cl::opt<ActionType> action(
    llvm::cl::desc("Action to perform:"),
    llvm::cl::values(clEnumValN(GenRustBindingDecls, "gen-rust-binding-decls",
                                "Generate Rust binding declarations")),
    llvm::cl::values(clEnumValN(GenCXXBindingDecls, "gen-cxx-binding-decls",
                                "Generate C++ BIR binding declarations")),
    llvm::cl::values(clEnumValN(GenCXXBindingDefs, "gen-cxx-binding-defs",
                                "Generate C++ BIR binding definitions")));

bool BIRTableGenMain(llvm::raw_ostream &OS, const llvm::RecordKeeper &Records) {
  switch (action) {
  case GenRustBindingDecls:
    bir::emitRustBindingDecls(Records, OS);
    break;
  case GenCXXBindingDecls:
    bir::emitCXXBindingDecls(Records, OS);
    break;
  case GenCXXBindingDefs:
    bir::emitCXXBindingDefs(Records, OS);
    break;
  }

  return false;
}

int main(int argc, char **argv) {
  llvm::sys::PrintStackTraceOnErrorSignal(argv[0]);
  llvm::PrettyStackTraceProgram X(argc, argv);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  llvm::llvm_shutdown_obj Y;

  return llvm::TableGenMain(argv[0], &BIRTableGenMain);
}
