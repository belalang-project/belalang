#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "belalang/BRT/BRT.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGPREPARERUNTIMEPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace belalang;

static bool hasBRTInitCall(bir::FuncOp mainFunc) {
  bool found = false;
  mainFunc.walk([&found](bir::CallOp callOp) {
    if (callOp.getCallee() == llvm::StringRef(brt::BRT_INIT)) {
      found = true;
      return mlir::WalkResult::interrupt();
    }
    return mlir::WalkResult::advance();
  });
  return found;
}

static void insertBRTInitCall(mlir::Operation *op) {
  mlir::ModuleOp module = dyn_cast_or_null<mlir::ModuleOp>(op);
  assert(module);

  mlir::OpBuilder builder(module.getContext());

  auto mainFunc = module.lookupSymbol<bir::FuncOp>("main");
  if (!mainFunc || mainFunc.isExternal())
    return;

  if (!module.lookupSymbol(brt::BRT_INIT)) {
    mlir::OpBuilder::InsertionGuard g(builder);
    builder.setInsertionPointToStart(module.getBody());
    auto funcType = mlir::FunctionType::get(builder.getContext(), {}, {});
    bir::FuncOp::create(builder, builder.getUnknownLoc(), brt::BRT_INIT,
                        funcType);
  }

  if (!hasBRTInitCall(mainFunc)) {
    mlir::OpBuilder::InsertionGuard g(builder);
    builder.setInsertionPointToStart(&mainFunc.getBody().front());
    auto callee = FlatSymbolRefAttr::get(builder.getContext(), brt::BRT_INIT);
    bir::CallOp::create(builder, builder.getUnknownLoc(), callee, {}, {});
  }
}

} // namespace

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

struct BelalangPrepareRuntimePass
    : public impl::BelalangPrepareRuntimePassBase<BelalangPrepareRuntimePass> {
  using impl::BelalangPrepareRuntimePassBase<
      BelalangPrepareRuntimePass>::BelalangPrepareRuntimePassBase;

  void runOnOperation() override { insertBRTInitCall(getOperation()); }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangPrepareRuntimePass() {
  return std::make_unique<BelalangPrepareRuntimePass>();
}
