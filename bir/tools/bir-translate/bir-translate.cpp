#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Target/LLVMIR/Dialect/Builtin/BuiltinToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Dialect/LLVMIR/LLVMToLLVMIRTranslation.h"
#include "mlir/Target/LLVMIR/Export.h"
#include "mlir/Tools/mlir-translate/MlirTranslateMain.h"
#include "mlir/Tools/mlir-translate/Translation.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

int main(int argc, char **argv) {
  // Custom MLIR to LLVMIR translation. mlir-translate registers all dialects
  // that can be translated into LLVMIR. Since we're not using all dialects;
  // only the LLVM dialect, we're making this custom translation.
  mlir::TranslateFromMLIRRegistration registration(
      "mlir-to-llvmir", "Translate LLVM dialect to LLVMIR",
      [](mlir::Operation *op, mlir::raw_ostream &output) {
        llvm::LLVMContext llvmContext;
        auto llvmModule = mlir::translateModuleToLLVMIR(op, llvmContext);
        if (!llvmModule)
          return mlir::failure();

        llvmModule->print(output, nullptr);
        return mlir::success();
      },
      [](mlir::DialectRegistry &registry) {
        registry.insert<mlir::LLVM::LLVMDialect>();
        mlir::registerLLVMDialectTranslation(registry);
        mlir::registerBuiltinDialectTranslation(registry);
      });

  mlir::TranslateFromMLIRRegistration BIRToLLVMIR(
      "bir-to-llvmir", "Translate BIR dialect directly to LLVMIR",
      [](mlir::Operation *op, mlir::raw_ostream &output) {
        mlir::PassManager pm(op->getContext());
        pm.addPass(belalang::bir::createBelalangBIRToLLVMPass());
        if (mlir::failed(pm.run(op)))
          return mlir::failure();

        llvm::LLVMContext llvmContext;
        auto llvmModule = mlir::translateModuleToLLVMIR(op, llvmContext);
        if (!llvmModule)
          return mlir::failure();

        llvmModule->print(output, nullptr);
        return mlir::success();
      },
      [](mlir::DialectRegistry &registry) {
        registry.insert<mlir::LLVM::LLVMDialect, belalang::bir::BIRDialect>();
        mlir::registerLLVMDialectTranslation(registry);
        mlir::registerBuiltinDialectTranslation(registry);
      });

  return mlir::failed(
      mlir::mlirTranslateMain(argc, argv, "BIR Translation Tool"));
}
