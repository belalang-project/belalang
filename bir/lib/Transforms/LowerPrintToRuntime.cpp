#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "belalang/BRT/BRT.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGLOWERPRINTTORUNTIMEPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace belalang;
using namespace belalang::bir;

struct PrintOpLowering : public mlir::OpRewritePattern<PrintOp> {
  using OpRewritePattern<PrintOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(PrintOp op, mlir::PatternRewriter &rewriter) const override {
    auto value = op.getValue();
    mlir::ModuleOp mod = op->getParentOfType<mlir::ModuleOp>();

    if (auto v = mlir::dyn_cast<IntType>(value.getType())) {
      bir::FuncOp f = mod.lookupSymbol<bir::FuncOp>(brt::BRT_PRINT_INT);

      if (!f) {
        mlir::Type ty = rewriter.getType<bir::IntType>();
        mlir::FunctionType funcType = rewriter.getFunctionType({ty}, {});

        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());

        f = bir::FuncOp::create(rewriter, op.getLoc(), brt::BRT_PRINT_INT,
                                funcType);
        f.setPrivate();
      }

      rewriter.replaceOpWithNewOp<bir::CallOp>(op, f, op->getOperands());
      return mlir::success();
    }

    if (auto v = mlir::dyn_cast<bir::FloatType>(value.getType())) {
      bir::FuncOp f = mod.lookupSymbol<bir::FuncOp>(brt::BRT_PRINT_FLOAT);

      if (!f) {
        mlir::Type ty = rewriter.getType<bir::FloatType>();
        mlir::FunctionType funcType = rewriter.getFunctionType({ty}, {});

        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());

        f = bir::FuncOp::create(rewriter, op.getLoc(), brt::BRT_PRINT_FLOAT,
                                funcType);
        f.setPrivate();
      }

      rewriter.replaceOpWithNewOp<bir::CallOp>(op, f, op->getOperands());
      return mlir::success();
    }

    if (auto v = mlir::dyn_cast<bir::StringType>(value.getType())) {
      bir::FuncOp f = mod.lookupSymbol<bir::FuncOp>(brt::BRT_PRINT_STRING);

      if (!f) {
        mlir::Type ty = rewriter.getType<bir::StringType>();
        mlir::FunctionType funcType = rewriter.getFunctionType({ty}, {});

        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());

        f = bir::FuncOp::create(rewriter, op.getLoc(), brt::BRT_PRINT_STRING,
                                funcType);
        f.setPrivate();
      }

      rewriter.replaceOpWithNewOp<bir::CallOp>(op, f, op->getOperands());
      return mlir::success();
    }

    if (auto v = mlir::dyn_cast<bir::BoolType>(value.getType())) {
      bir::FuncOp f = mod.lookupSymbol<bir::FuncOp>(brt::BRT_PRINT_BOOL);

      if (!f) {
        mlir::Type ty = rewriter.getType<bir::BoolType>();
        mlir::FunctionType funcType = rewriter.getFunctionType({ty}, {});

        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());

        f = bir::FuncOp::create(rewriter, op.getLoc(), brt::BRT_PRINT_BOOL,
                                funcType);
        f.setPrivate();
      }

      rewriter.replaceOpWithNewOp<bir::CallOp>(op, f, op->getOperands());
      return mlir::success();
    }

    return mlir::failure();
  }
};

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

void belalang::bir::populateBelalangLowerPrintToRuntimePatterns(
    mlir::RewritePatternSet &patterns) {
  patterns.add<PrintOpLowering>(patterns.getContext());
}

// -----------------------------------------------------------------------------
// The Pass
// -----------------------------------------------------------------------------

struct BelalangLowerPrintToRuntimePass
    : public impl::BelalangLowerPrintToRuntimePassBase<BelalangLowerPrintToRuntimePass> {
  using impl::BelalangLowerPrintToRuntimePassBase<
      BelalangLowerPrintToRuntimePass>::BelalangLowerPrintToRuntimePassBase;

  void runOnOperation() override {
    insertBRTInitCall(getOperation());

    mlir::RewritePatternSet patterns(&getContext());
    populateBelalangLowerPrintToRuntimePatterns(patterns);

    if (mlir::failed(
            mlir::applyPatternsGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangLowerPrintToRuntimePass() {
  return std::make_unique<BelalangLowerPrintToRuntimePass>();
}
