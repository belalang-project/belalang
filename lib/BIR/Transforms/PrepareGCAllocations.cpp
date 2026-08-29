#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/IR/Dominance.h"

namespace mlir {
#define GEN_PASS_DEF_BELALANGPREPAREGCALLOCATIONSPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {
using namespace mlir;
using namespace belalang;

bool isLiveAcross(bir::AllocHeapOp alloc, Value value,
                  DominanceInfo &di) {
  if (!di.properlyDominates(value, alloc))
    return false;

  return llvm::any_of(value.getUses(), [&](OpOperand &use) {
    return di.dominates(alloc.getOperation(), use.getOwner());
  });
}

struct BelalangPrepareGCAllocationsPass
    : public mlir::impl::BelalangPrepareGCAllocationsPassBase<
          BelalangPrepareGCAllocationsPass> {
  using mlir::impl::BelalangPrepareGCAllocationsPassBase<
      BelalangPrepareGCAllocationsPass>::BelalangPrepareGCAllocationsPassBase;

  void runOnOperation() override {
    getOperation()->walk([&](bir::FuncOp fn) {
      DominanceInfo dominance(fn);

      // Find all allocations in this function body.
      llvm::SmallVector<bir::AllocHeapOp> allocations;
      fn.walk([&](bir::AllocHeapOp alloc) { allocations.push_back(alloc); });

      for (bir::AllocHeapOp alloc : allocations) {
        // Find all roots.
        llvm::SmallVector<Value> roots;
        fn.walk([&](Operation *op) {
          for (Value result : op->getResults()) {
            if (isa<bir::RefType>(result.getType()) &&
                isLiveAcross(alloc, result, dominance))
              roots.push_back(result);
          }
        });
        for (Block &block : fn.getBody()) {
          for (BlockArgument argument : block.getArguments()) {
            if (isa<bir::RefType>(argument.getType()) &&
                isLiveAcross(alloc, argument, dominance))
              roots.push_back(argument);
          }
        }

        // We're appending the AllocHeapOp with roots types.
        llvm::SmallVector<Type> resultTypes = {alloc.getResult().getType()};
        llvm::append_range(resultTypes, ValueRange(roots).getTypes());

        // Make a new AllocHeapOp to replace the current one.
        OpBuilder builder(alloc);
        auto prepared = bir::AllocHeapOp::create(
            builder, alloc.getLoc(), resultTypes, ValueRange(roots));

        for (auto [root, relocated] :
             llvm::zip_equal(roots, prepared.getRelocatedRoots())) {
          // Snapshot of the root's uses.
          auto uses = llvm::map_to_vector(root.getUses(),
                                          [](OpOperand &use) { return &use; });

          // Update the uses.
          for (OpOperand *use : uses) {
            Operation *owner = use->getOwner();
            if (owner != prepared.getOperation() &&
                dominance.dominates(alloc.getOperation(), owner))
              use->set(relocated);
          }
        }

        // Replace the old unprepared AllocHeapOp with the new prepared one.
        alloc.getResult().replaceAllUsesWith(prepared.getResult());
        alloc.erase();
      }
    });
  }
};

} // namespace

std::unique_ptr<mlir::Pass>
belalang::bir::createBelalangPrepareGCAllocationsPass() {
  return std::make_unique<BelalangPrepareGCAllocationsPass>();
}
