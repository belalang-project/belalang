#include "belalang/BRT/BRT.h"
#include "belalang/BIR/IR/BIRDialect.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/PatternMatch.h"

namespace belalang {

namespace bir {

namespace {
struct PrintOpLowering : public mlir::OpRewritePattern<PrintOp> {
  using OpRewritePattern<PrintOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(PrintOp op, mlir::PatternRewriter &rewriter) const override {
    auto value = op.getValue();
    mlir::ModuleOp mod = op->getParentOfType<mlir::ModuleOp>();

    if (auto v = mlir::dyn_cast<IntType>(value.getType())) {
      mlir::func::FuncOp f =
          mod.lookupSymbol<mlir::func::FuncOp>(brt::BRT_PRINT_INT);

      if (!f) {
        mlir::Type ty = rewriter.getType<IntType>();
        mlir::FunctionType funcType = rewriter.getFunctionType({ty}, {});

        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());

        f = mlir::func::FuncOp::create(rewriter, op.getLoc(),
                                       brt::BRT_PRINT_INT, funcType);
        f.setPrivate();
      }

      rewriter.replaceOpWithNewOp<mlir::func::CallOp>(op, f, op->getOperands());
      return mlir::success();
    }

    if (auto v = mlir::dyn_cast<FloatType>(value.getType())) {
      mlir::func::FuncOp f =
          mod.lookupSymbol<mlir::func::FuncOp>(brt::BRT_PRINT_FLOAT);

      if (!f) {
        mlir::Type ty = rewriter.getType<FloatType>();
        mlir::FunctionType funcType = rewriter.getFunctionType({ty}, {});

        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());

        f = mlir::func::FuncOp::create(rewriter, op.getLoc(),
                                       brt::BRT_PRINT_FLOAT, funcType);
        f.setPrivate();
      }

      rewriter.replaceOpWithNewOp<mlir::func::CallOp>(op, f, op->getOperands());
      return mlir::success();
    }

    return mlir::failure();
  }
};
} // namespace

void populateBelalangRuntimizePatterns(mlir::RewritePatternSet &patterns) {
  patterns.add<PrintOpLowering>(patterns.getContext());
}

} // namespace bir
} // namespace belalang
