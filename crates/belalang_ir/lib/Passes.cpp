#include "belalang_ir/Passes.h"
#include "belalang_ir/IR/BIRDialect.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Transforms/GreedyPatternRewriteDriver.h"

namespace bir {

#define GEN_PASS_DEF_BELALANGCONSTANTSPASS
#include "belalang_ir/Passes.h.inc"

#define GEN_PASS_DEF_BELALANGLOWERPRINTPASS
#include "belalang_ir/Passes.h.inc"

namespace {
struct ConstantOpLowering : public mlir::OpRewritePattern<ConstantOp> {
  using OpRewritePattern<ConstantOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(ConstantOp op,
                  mlir::PatternRewriter &rewriter) const override {
    auto value = op.getValue();
    if (auto intAttr = mlir::dyn_cast<mlir::IntegerAttr>(value)) {
      auto newOp = mlir::arith::ConstantOp::create(
          rewriter, op.getLoc(), rewriter.getI32IntegerAttr(intAttr.getInt()));
      rewriter.replaceOpWithNewOp<mlir::UnrealizedConversionCastOp>(
          op, op.getType(), newOp.getResult());
      return mlir::success();
    }
    if (auto floatAttr = mlir::dyn_cast<mlir::FloatAttr>(value)) {
      auto newOp = mlir::arith::ConstantOp::create(
          rewriter, op.getLoc(),
          rewriter.getF32FloatAttr(floatAttr.getValueAsDouble()));
      rewriter.replaceOpWithNewOp<mlir::UnrealizedConversionCastOp>(
          op, op.getType(), newOp.getResult());
      return mlir::success();
    }
    return mlir::failure();
  }
};

struct BelalangConstantsPass
    : public impl::BelalangConstantsPassBase<BelalangConstantsPass> {
  using impl::BelalangConstantsPassBase<
      BelalangConstantsPass>::BelalangConstantsPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    patterns.add<ConstantOpLowering>(&getContext());

    if (mlir::failed(
            mlir::applyPatternsGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

struct PrintOpLowering : public mlir::OpRewritePattern<PrintOp> {
  using OpRewritePattern<PrintOp>::OpRewritePattern;

  mlir::LogicalResult
  matchAndRewrite(PrintOp op, mlir::PatternRewriter &rewriter) const override {
    auto value = op.getValue();
    mlir::ModuleOp mod = op->getParentOfType<mlir::ModuleOp>();

    if (auto v = mlir::dyn_cast<IntType>(value.getType())) {
      mlir::func::FuncOp f =
          mod.lookupSymbol<mlir::func::FuncOp>("belalang_print_int");

      if (!f) {
        mlir::Type ty = rewriter.getType<IntType>();
        mlir::FunctionType funcType = rewriter.getFunctionType({ty}, {});

        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());

        f = mlir::func::FuncOp::create(rewriter, op.getLoc(),
                                       "belalang_print_int", funcType);
        f.setPrivate();
      }

      rewriter.replaceOpWithNewOp<mlir::func::CallOp>(op, f, op->getOperands());
      return mlir::success();
    }

    if (auto v = mlir::dyn_cast<FloatType>(value.getType())) {
      mlir::func::FuncOp f =
          mod.lookupSymbol<mlir::func::FuncOp>("belalang_print_float");

      if (!f) {
        mlir::Type ty = rewriter.getType<FloatType>();
        mlir::FunctionType funcType = rewriter.getFunctionType({ty}, {});

        mlir::OpBuilder::InsertionGuard guard(rewriter);
        rewriter.setInsertionPointToStart(mod.getBody());

        f = mlir::func::FuncOp::create(rewriter, op.getLoc(),
                                       "belalang_print_float", funcType);
        f.setPrivate();
      }

      rewriter.replaceOpWithNewOp<mlir::func::CallOp>(op, f, op->getOperands());
      return mlir::success();
    }

    return mlir::failure();
  }
};

struct BelalangLowerPrintPass
    : public impl::BelalangLowerPrintPassBase<BelalangLowerPrintPass> {
  using impl::BelalangLowerPrintPassBase<
      BelalangLowerPrintPass>::BelalangLowerPrintPassBase;

  void runOnOperation() override {
    mlir::RewritePatternSet patterns(&getContext());
    patterns.add<PrintOpLowering>(&getContext());

    if (mlir::failed(
            mlir::applyPatternsGreedily(getOperation(), std::move(patterns)))) {
      signalPassFailure();
    }
  }
};

} // namespace

} // namespace bir
