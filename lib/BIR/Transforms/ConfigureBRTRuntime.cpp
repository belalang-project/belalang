#include "belalang/BIR/BRTUtils.h"
#include "belalang/BIR/Transforms/Passes.h"
#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/IR/BuiltinOps.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGCONFIGUREBRTRUNTIMEPASS
#include "belalang/BIR/Transforms/Passes.h.inc"
} // namespace mlir

namespace {

struct BelalangConfigureBRTRuntimePass
    : public mlir::impl::BelalangConfigureBRTRuntimePassBase<
          BelalangConfigureBRTRuntimePass> {
  using mlir::impl::BelalangConfigureBRTRuntimePassBase<
      BelalangConfigureBRTRuntimePass>::BelalangConfigureBRTRuntimePassBase;

  void runOnOperation() override {
    mlir::ModuleOp module = getOperation();
    mlir::Builder builder(module.getContext());
    module->setAttr(mlir::gc::kRuntimeAllocAttrName,
                    builder.getStringAttr(kGCAllocLayout));
    module->setAttr(mlir::gc::kRuntimePushRootsAttrName,
                    builder.getStringAttr(kGCPushRoots));
    module->setAttr(mlir::gc::kRuntimePopRootsAttrName,
                    builder.getStringAttr(kGCPopRoots));
    insertBRTInitCall(module);
  }
};

} // namespace

std::unique_ptr<mlir::Pass>
belalang::bir::createBelalangConfigureBRTRuntimePass() {
  return std::make_unique<BelalangConfigureBRTRuntimePass>();
}
