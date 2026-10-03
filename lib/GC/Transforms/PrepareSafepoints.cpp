#include "mlir/Dialect/GC/IR/GC.h"
#include "mlir/Dialect/GC/Passes.h"
#include "mlir/IR/Dominance.h"
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

static bool isLiveAcross(Value value, gc::AllocOp alloc,
                         DominanceInfo &dominance) {
  if (!dominance.properlyDominates(value, alloc))
    return false;

  return llvm::any_of(value.getUses(), [&](OpOperand &use) {
    return use.getOwner() != alloc.getOperation() &&
           dominance.dominates(alloc.getOperation(), use.getOwner());
  });
}

static Value findLiveDerivedPointer(FunctionOpInterface fn, gc::AllocOp alloc,
                                    DominanceInfo &dominance) {
  Value liveDerivedPointer;
  fn->walk([&](Operation *op) {
    if (liveDerivedPointer)
      return;
    for (Value result : op->getResults()) {
      if (isa<gc::DerivedPtrType>(result.getType()) &&
          isLiveAcross(result, alloc, dominance)) {
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
          isLiveAcross(argument, alloc, dominance))
        return argument;
    }
  }

  return {};
}

struct GCPrepareSafepoints final
    : public impl::GCPrepareSafepointsPassBase<GCPrepareSafepoints> {
  using impl::GCPrepareSafepointsPassBase<
      GCPrepareSafepoints>::GCPrepareSafepointsPassBase;

  void runOnOperation() override {
    bool foundUnsupportedDerivedPointer = false;
    getOperation()->walk([&](FunctionOpInterface fn) {
      if (foundUnsupportedDerivedPointer)
        return;

      DominanceInfo dominance(fn);

      llvm::SmallVector<gc::AllocOp> allocs;
      fn->walk([&](gc::AllocOp alloc) { allocs.push_back(alloc); });

      for (gc::AllocOp alloc : allocs) {
        if (findLiveDerivedPointer(fn, alloc, dominance)) {
          alloc.emitOpError(
              "does not support derived pointers live across safepoints");
          foundUnsupportedDerivedPointer = true;
          return;
        }

        llvm::SmallSetVector<Value, 8> roots;

        for (Value root : alloc.getRoots())
          roots.insert(root);

        fn->walk([&](Operation *op) {
          for (Value result : op->getResults()) {
            if (isa<gc::PtrType>(result.getType()) &&
                isLiveAcross(result, alloc, dominance))
              roots.insert(result);
          }
        });

        for (Block &block : fn.getFunctionBody()) {
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
            builder, alloc.getLoc(), resultTypes, roots.getArrayRef(),
            alloc->getAttrs());

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

        for (auto [oldResult, newResult] :
             llvm::zip(alloc->getResults(), prepared->getResults()))
          oldResult.replaceAllUsesWith(newResult);
        alloc.erase();
      }
    });

    if (foundUnsupportedDerivedPointer)
      signalPassFailure();
  }
};

} // namespace
