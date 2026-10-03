#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/GC/Passes.h"
#include "mlir/IR/Dominance.h"
#include "mlir/IR/PatternMatch.h"
#include "mlir/IR/ValueRange.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"

namespace mlir {
#define GEN_PASS_DEF_GCPREPARESAFEPOINTSPASS
#include "mlir/Dialect/GC/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;

static bool isLiveAcross(Value value, gc::SafepointOpInterface safepoint,
                         DominanceInfo &dominance) {
  Operation *op = safepoint.getOperation();

  if (!dominance.properlyDominates(value, op))
    return false;

  return llvm::any_of(value.getUses(), [&](OpOperand &use) {
    return use.getOwner() != op && dominance.dominates(op, use.getOwner());
  });
}

static Value findLiveDerivedPointer(FunctionOpInterface fn,
                                    gc::SafepointOpInterface safepoint,
                                    DominanceInfo &dominance) {
  Value liveDerivedPointer;
  fn->walk([&](Operation *op) {
    if (liveDerivedPointer)
      return;
    for (Value result : op->getResults()) {
      if (isa<gc::DerivedPtrType>(result.getType()) &&
          isLiveAcross(result, safepoint, dominance)) {
        liveDerivedPointer = result;
        return;
      }
    }
  });

  if (liveDerivedPointer)
    return liveDerivedPointer;

  for (Block &block : fn.getFunctionBody()) {
    for (BlockArgument argument : block.getArguments()) {
      if (isa<gc::DerivedPtrType>(argument.getType()) &&
          isLiveAcross(argument, safepoint, dominance))
        return argument;
    }
  }

  return {};
}

struct GCPrepareSafepoints final
    : impl::GCPrepareSafepointsPassBase<GCPrepareSafepoints> {
  using Base::Base;

  void runOnOperation() override {
    bool failedPreparation = false;

    getOperation()->walk([&](FunctionOpInterface fn) {
      if (failedPreparation)
        return;

      DominanceInfo dominance(fn);
      IRRewriter rewriter(&getContext());

      llvm::SmallVector<gc::SafepointOpInterface> safepoints;
      fn->walk([&](Operation *op) {
        if (auto safepoint = dyn_cast<gc::SafepointOpInterface>(op))
          safepoints.push_back(safepoint);
      });

      for (gc::SafepointOpInterface safepoint : safepoints) {
        Operation *oldOp = safepoint.getOperation();

        if (findLiveDerivedPointer(fn, safepoint, dominance)) {
          oldOp->emitOpError(
              "does not support derived pointers live across safepoints");
          failedPreparation = true;
          return;
        }

        llvm::SmallSetVector<Value, 8> roots;

        for (Value root : safepoint.getRoots())
          roots.insert(root);

        fn->walk([&](Operation *op) {
          for (Value result : op->getResults()) {
            if (isa<gc::PtrType>(result.getType()) &&
                isLiveAcross(result, safepoint, dominance))
              roots.insert(result);
          }
        });

        for (Block &block : fn.getFunctionBody()) {
          for (BlockArgument argument : block.getArguments()) {
            if (isa<gc::PtrType>(argument.getType()) &&
                isLiveAcross(argument, safepoint, dominance))
              roots.insert(argument);
          }
        }

        FailureOr<gc::SafepointOpInterface>
            prepared = safepoint.rebuildWithRoots(rewriter,
                                                  roots.getArrayRef());

        if (failed(prepared)) {
          oldOp->emitOpError("failed to rebuild safepoint with live roots");
          failedPreparation = true;
          return;
        }

        Operation *preparedOp = prepared->getOperation();

        for (auto [root, reloc] : llvm::zip_equal(
                 roots.getArrayRef(), prepared->getRelocatedRoots())) {
          for (OpOperand *use : llvm::map_to_vector(
                   root.getUses(), [](OpOperand &use) { return &use; })) {
            Operation *owner = use->getOwner();
            if (owner != oldOp && owner != preparedOp &&
                dominance.dominates(oldOp, owner))
              use->set(reloc);
          }
        }

        assert(oldOp->getNumResults() <= preparedOp->getNumResults() &&
               "rebuilt safepoint dropped existing results");

        for (auto [oldResult, newResult] :
             llvm::zip(oldOp->getResults(), preparedOp->getResults()))
          rewriter.replaceAllUsesWith(oldResult, newResult);
        rewriter.eraseOp(oldOp);
      }
    });

    if (failedPreparation)
      signalPassFailure();
  }
};

} // namespace
