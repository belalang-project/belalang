#include "TableGenBackends.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Main.h"

using namespace belalang;

enum ActionType {
  GenCXXBindings,
  GenCXXBindingsDecl,
  GenCXXBindingsDefs,
};

llvm::cl::opt<ActionType> action(
    llvm::cl::desc("Action to perform:"),
    llvm::cl::values(clEnumValN(GenCXXBindings, "gen-rust-bindings",
                                "Generate cxx.rs Rust Bindings")),
    llvm::cl::values(clEnumValN(GenCXXBindingsDecl, "gen-cxx-bindings-decl",
                                "Generate C++ BIR binding declarations")),
    llvm::cl::values(clEnumValN(GenCXXBindingsDefs, "gen-cxx-bindings-defs",
                                "Generate C++ BIR binding definitions")));

bool BIRTableGenMain(llvm::raw_ostream &OS, const llvm::RecordKeeper &Records) {
  switch (action) {
  case GenCXXBindings:
    bir::emitRustBindings(Records, OS);
    break;
  case GenCXXBindingsDecl:
    bir::emitCXXBindingsDecl(Records, OS);
    break;
  case GenCXXBindingsDefs:
    bir::emitCXXBindingsDefs(Records, OS);
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
