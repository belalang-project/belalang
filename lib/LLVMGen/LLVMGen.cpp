#include "belalang/LLVMGen/LLVMGen.h"
#include "belalang/BIR/Passes.h"
#include "belalang/LLVMGen/LLVMGen.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Export.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/Instrumentation/ThreadSanitizer.h"

namespace belalang {
namespace llvmgen {

LLVMGen::LLVMGen(uintptr_t ptr) {
  auto op = reinterpret_cast<mlir::ModuleOp *>(ptr);

  // Convert BIR dialect to LLVM Dialect.
  mlir::PassManager pm(op->getContext());
  pm.addPass(bir::createBelalangBIRToLLVMPass());
  assert(mlir::succeeded(pm.run(*op)) && "conversion to LLVM dialect failed.");

  // Translate LLVM Dialect to LLVM IR.
  mlir::DialectRegistry registry;
  mlir::registerLLVMDialectTranslation(registry);
  mlir::registerBuiltinDialectTranslation(registry);
  op->getContext()->appendDialectRegistry(registry);

  llvmModule = mlir::translateModuleToLLVMIR(*op, llvmCtx);
  assert(llvmModule && "translation to LLVM IR failed.");
}

void LLVMGen::compileObjFile(std::string outfile, SanitizerKind san) const {
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  const auto triple = llvm::Triple(llvm::sys::getDefaultTargetTriple());
  llvmModule->setTargetTriple(triple);

  std::string error;
  const llvm::Target *target =
      llvm::TargetRegistry::lookupTarget(triple, error);

  if (!target)
    return;

  llvm::StringRef cpu = "generic";
  llvm::StringRef features = "";
  llvm::TargetOptions opt;
  auto rm = llvm::Reloc::PIC_;

  auto tm = target->createTargetMachine(triple, cpu, features, opt, rm);
  llvmModule->setDataLayout(tm->createDataLayout());

  if (san != SanitizerKind::None) {
    llvm::LoopAnalysisManager lam;
    llvm::FunctionAnalysisManager fam;
    llvm::CGSCCAnalysisManager cgam;
    llvm::ModuleAnalysisManager mam;

    llvm::PassBuilder pb;
    pb.registerModuleAnalyses(mam);
    pb.registerCGSCCAnalyses(cgam);
    pb.registerFunctionAnalyses(fam);
    pb.registerLoopAnalyses(lam);
    pb.crossRegisterProxies(lam, fam, cgam, mam);

    llvm::ModulePassManager mpm;
    if (san == SanitizerKind::Thread) {
      mpm.addPass(llvm::ModuleThreadSanitizerPass());
      mpm.addPass(
          llvm::createModuleToFunctionPassAdaptor(llvm::ThreadSanitizerPass()));
    }

    mpm.run(*llvmModule, mam);
  }

  std::error_code ec;
  llvm::raw_fd_ostream dest(llvm::StringRef(outfile.data(), outfile.size()), ec,
                            llvm::sys::fs::OF_None);
  if (ec)
    return;

  llvm::legacy::PassManager pm;
  if (tm->addPassesToEmitFile(pm, dest, nullptr,
                              llvm::CodeGenFileType::ObjectFile))
    return;

  pm.run(*llvmModule);

  dest.flush();
}

std::string LLVMGen::dumpToString() const {
  std::string s;
  llvm::raw_string_ostream os(s);
  llvmModule->print(os, nullptr);
  return os.str();
}

} // namespace llvmgen
} // namespace belalang
