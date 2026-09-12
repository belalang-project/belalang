#include "belalang/GCIR/IR/GCIR.h"
#include "belalang/GCIR/Transforms/Passes.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/Dominance.h"
#include "mlir/IR/ValueRange.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"

namespace mlir {
#define GEN_PASS_DEF_GCIRPREPAREGCSAFEPOINTSPASS
#include "belalang/GCIR/Transforms/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;
using namespace belalang;

static bool isLiveAcross(Value value, gc::AllocOp alloc,
                         DominanceInfo &dominance) {
  if (!dominance.properlyDominates(value, alloc))
    return false;

  return llvm::any_of(value.getUses(), [&](OpOperand &use) {
    return use.getOwner() != alloc.getOperation() &&
           dominance.dominates(alloc.getOperation(), use.getOwner());
  });
}

struct GCIRPrepareGCSafepoints
    : public impl::GCIRPrepareGCSafepointsPassBase<GCIRPrepareGCSafepoints> {
  using impl::GCIRPrepareGCSafepointsPassBase<
      GCIRPrepareGCSafepoints>::GCIRPrepareGCSafepointsPassBase;

  void runOnOperation() override {
    getOperation()->walk([&](func::FuncOp fn) {
      DominanceInfo dominance(fn);

      llvm::SmallVector<gc::AllocOp> allocs;
      fn.walk([&](gc::AllocOp alloc) { allocs.push_back(alloc); });

      for (gc::AllocOp alloc : allocs) {
        llvm::SmallSetVector<Value, 8> roots;

        for (Value root : alloc.getRoots())
          roots.insert(root);

        fn.walk([&](Operation *op) {
          for (Value result : op->getResults()) {
            if (isa<gc::PtrType>(result.getType()) &&
                isLiveAcross(result, alloc, dominance))
              roots.insert(result);
          }
        });

        for (Block &block : fn.getBlocks()) {
          for (BlockArgument argument : block.getArguments()) {
            if (isa<gc::PtrType>(argument.getType()) &&
                isLiveAcross(argument, alloc, dominance))
              roots.insert(argument);
          }
        }

        llvm::SmallVector<Type> resultTypes = {alloc.getResult().getType()};
        llvm::append_range(resultTypes,
                           ValueRange(roots.getArrayRef()).getTypes());

        OpBuilder builder(alloc);
        gc::AllocOp prepared = gc::AllocOp::create(
            builder, alloc.getLoc(), resultTypes, roots.getArrayRef());

        for (auto [root, reloc] : llvm::zip_equal(
                 roots.getArrayRef(), prepared.getRelocatedRoots())) {
          for (OpOperand *use : llvm::map_to_vector(
                   root.getUses(), [](OpOperand &use) { return &use; })) {
            Operation *owner = use->getOwner();
            if (owner != alloc.getOperation() &&
                owner != prepared.getOperation() &&
                dominance.dominates(alloc.getOperation(), owner))
              use->set(reloc);
          }
        }

        alloc.getResult().replaceAllUsesWith(prepared.getResult());
        alloc.erase();
      }
    });
  }
};

} // namespace

void mlir::registerGCIRPrepareGCSafepointsPass() {
  mlir::registerPass([]() -> std::unique_ptr<mlir::Pass> {
    return mlir::createGCIRPrepareGCSafepointsPass();
  });
}
