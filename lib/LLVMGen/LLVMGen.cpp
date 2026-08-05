#include "belalang/BIR/Passes.h"
#include "belalang/LLVMGen/LLVMGen.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Export.h"

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

std::string LLVMGen::dumpToString() const {
  std::string s;
  llvm::raw_string_ostream os(s);
  llvmModule->print(os, nullptr);
  return os.str();
}

} // namespace llvmgen
} // namespace belalang
