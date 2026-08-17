#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Dominance.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGINSERTSTACKMAPSPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace mlir;
using namespace belalang;
using namespace belalang::bir;

struct BelalangInsertStackMapsPass
    : public mlir::impl::BelalangInsertStackMapsPassBase<
          BelalangInsertStackMapsPass> {
  using mlir::impl::BelalangInsertStackMapsPassBase<
      BelalangInsertStackMapsPass>::BelalangInsertStackMapsPassBase;

  void runOnOperation() override {
    uint64_t nextId = 0;

    getOperation()->walk([&](bir::FuncOp func) {
      mlir::DominanceInfo dom(func);

      llvm::SmallVector<mlir::Value> refValues;
      func.walk([&](mlir::Operation *op) {
        for (auto res : op->getResults()) {
          if (mlir::isa<bir::RefType>(res.getType()))
            refValues.push_back(res);
        }
      });
      for (auto &block : func.getBody().getBlocks()) {
        for (auto arg : block.getArguments()) {
          if (mlir::isa<bir::RefType>(arg.getType()))
            refValues.push_back(arg);
        }
      }

      llvm::SmallVector<bir::AllocHeapOp> allocs;
      func.walk([&](bir::AllocHeapOp alloc) { allocs.push_back(alloc); });

      for (auto alloc : allocs) {
        llvm::SmallVector<mlir::Value> roots;
        for (auto v : refValues) {
          if (dom.properlyDominates(v, alloc))
            roots.push_back(v);
        }

        mlir::OpBuilder builder(alloc);
        bir::SafepointOp::create(builder, alloc.getLoc(), nextId++, roots);
      }
    });
  }
};

} // namespace

std::unique_ptr<mlir::Pass> belalang::bir::createBelalangInsertStackMapsPass() {
  return std::make_unique<BelalangInsertStackMapsPass>();
}
