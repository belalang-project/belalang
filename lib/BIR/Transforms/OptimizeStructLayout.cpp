#include "belalang/BIR/Transforms/OptimizeStructLayout.h"
#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/Interfaces/DataLayoutInterfaces.h"
#include <functional>
#include <numeric>

namespace mlir {
#define GEN_PASS_DEF_BELALANGOPTIMIZESTRUCTLAYOUTPASS
#include "belalang/BIR/Passes.h.inc"
} // namespace mlir

namespace {

using namespace mlir;
using namespace belalang;

struct OptimizeStructLayoutPass final
    : public mlir::impl::BelalangOptimizeStructLayoutPassBase<
          OptimizeStructLayoutPass> {
  using BelalangOptimizeStructLayoutPassBase::
      BelalangOptimizeStructLayoutPassBase;

  void runOnOperation() override {
    ModuleOp mod = cast<ModuleOp>(getOperation());
    DataLayout dl = DataLayout::closest(mod.getOperation());
    llvm::SmallSetVector<Type, 16> visited;

    std::function<void(Type)> collectType = [&](Type type) {
      if (!visited.insert(type))
        return;
      if (auto structType = dyn_cast<bir::StructType>(type)) {
        for (Type member : structType.getMembers())
          collectType(member);
        if (optimizeStructLayout(dl, structType).failed())
          return signalPassFailure();
      } else if (auto refType = dyn_cast<bir::RefType>(type)) {
        collectType(refType.getReferent());
      }
    };

    mod.walk([&](Operation *op) {
      for (Type type : op->getOperandTypes())
        collectType(type);
      for (Type type : op->getResultTypes())
        collectType(type);
      for (Region &region : op->getRegions())
        for (Block &block : region)
          for (BlockArgument argument : block.getArguments())
            collectType(argument.getType());
      if (auto func = dyn_cast<bir::FuncOp>(op))
        for (Type type : func.getFunctionType().getInputs())
          collectType(type);
      if (auto func = dyn_cast<bir::FuncOp>(op))
        for (Type type : func.getFunctionType().getResults())
          collectType(type);
    });
  }
};

} // namespace

namespace belalang {
namespace bir {

llvm::SmallVector<int32_t>
computeStructReorder(llvm::ArrayRef<StructMemberLayout> m) {
  // Initialize to identity.
  llvm::SmallVector<unsigned> perm(m.size());
  std::iota(perm.begin(), perm.end(), 0);

  // Sort based on alignment descending, then size descending.
  llvm::stable_sort(perm, [&](unsigned lhs, unsigned rhs) {
    if (m[lhs].alignment != m[rhs].alignment)
      return m[lhs].alignment > m[rhs].alignment;
    return m[lhs].size > m[rhs].size;
  });

  llvm::SmallVector<int32_t> invPerm(perm.size());
  for (unsigned i = 0; i < perm.size(); ++i)
    invPerm[perm[i]] = i;

  return invPerm;
}

mlir::LogicalResult optimizeStructLayout(const mlir::DataLayout &dl,
                                         StructType type) {
  llvm::SmallVector<StructMemberLayout> members;
  members.reserve(type.getMembers().size());

  for (mlir::Type member : type.getMembers()) {
    members.emplace_back(dl.getTypeABIAlignment(member),
                         dl.getTypeSize(member).getFixedValue());
  }

  return type.setReorder(computeStructReorder(members));
}

std::unique_ptr<mlir::Pass> createBelalangOptimizeStructLayoutPass() {
  return std::make_unique<OptimizeStructLayoutPass>();
}

} // namespace bir
} // namespace belalang
