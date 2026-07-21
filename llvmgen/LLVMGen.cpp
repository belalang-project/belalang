#include "belalang/LLVMGen/LLVMGen.h"

#include "belalang/BIR/Passes.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Export.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/Instrumentation/ThreadSanitizer.h"

using namespace llvm;

namespace belalang {
namespace llvmgen {

std::unique_ptr<LLVMGen> create_llvmgen(uintptr_t module_ptr) {
  return std::make_unique<LLVMGen>(
      reinterpret_cast<mlir::ModuleOp *>(module_ptr));
}

LLVMGen::LLVMGen(mlir::ModuleOp *op) {
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

rust::String LLVMGen::compile_object_file(rust::String out,
                                          SanitizerKind sanitizer) const {
  InitializeAllTargetInfos();
  InitializeAllTargets();
  InitializeAllTargetMCs();
  InitializeAllAsmParsers();
  InitializeAllAsmPrinters();

  const Triple triple = Triple(sys::getDefaultTargetTriple());
  llvmModule->setTargetTriple(triple);

  std::string error;
  const llvm::Target *target = TargetRegistry::lookupTarget(triple, error);
  if (!target)
    return rust::String(error);
  llvm::StringRef cpu = "generic";
  llvm::StringRef features = "";
  llvm::TargetOptions opt;
  auto rm = llvm::Reloc::PIC_;

  auto tm = target->createTargetMachine(triple, cpu, features, opt, rm);
  llvmModule->setDataLayout(tm->createDataLayout());

  if (sanitizer != SanitizerKind::None) {
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
    if (sanitizer == SanitizerKind::Thread) {
      mpm.addPass(llvm::ModuleThreadSanitizerPass());
      mpm.addPass(
          llvm::createModuleToFunctionPassAdaptor(llvm::ThreadSanitizerPass()));
    }

    mpm.run(*llvmModule, mam);
  }

  std::error_code ec;
  raw_fd_ostream dest(llvm::StringRef(out.data(), out.size()), ec);
  if (ec)
    return rust::String("failed to create destination");

  legacy::PassManager pm;
  if (tm->addPassesToEmitFile(pm, dest, nullptr, CodeGenFileType::ObjectFile))
    return rust::String("failed to add passes");

  pm.run(*llvmModule);

  dest.flush();
  return rust::String();
}

rust::String LLVMGen::dump_to_string() const {
  std::string s;
  llvm::raw_string_ostream os(s);
  llvmModule->print(os, nullptr);
  return rust::String(os.str());
}

} // namespace llvmgen
} // namespace belalang
