#include "belalang/BIR/Analysis/EscapeAnalysis.h"
#include "belalang/BIR/IR/BIR.h"
#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace belalang {
namespace bir {

// -----------------------------------------------------------------------------
// Escape Lattice
// -----------------------------------------------------------------------------

void EscapeLattice::print(llvm::raw_ostream &os) const {
  os << (escapes ? "escape" : "noescape");
}

mlir::ChangeResult EscapeLattice::markEscapes() {
  bool was = escapes;
  escapes = true;
  return was ? mlir::ChangeResult::NoChange : mlir::ChangeResult::Change;
}

mlir::ChangeResult
EscapeLattice::meet(const mlir::dataflow::AbstractSparseLattice &other) {
  auto lattice = reinterpret_cast<const EscapeLattice *>(&other);
  return lattice->escapes ? markEscapes() : mlir::ChangeResult::NoChange;
}

// -----------------------------------------------------------------------------
// Escape Analysis
// -----------------------------------------------------------------------------

mlir::LogicalResult
EscapeAnalysis::visitOperation(mlir::Operation *op,
                               llvm::ArrayRef<EscapeLattice *> operands,
                               llvm::ArrayRef<const EscapeLattice *> results) {
  if (mlir::isa<bir::ReturnOp>(op)) {
    for (EscapeLattice *operand : operands) {
      propagateIfChanged(operand, operand->markEscapes());
    }
  }

  if (mlir::isa<bir::VarLoadOp>(op) && !results.empty() &&
      results.front()->escapes) {
    propagateIfChanged(operands.front(), operands.front()->markEscapes());
  }

  return mlir::success();
}

void EscapeAnalysis::visitBranchOperand(mlir::OpOperand &operand) {
  mlir::Operation *op = operand.getOwner();
  assert(isa<RegionBranchOpInterface>(op) || isa<BranchOpInterface>(op) ||
         isa<RegionBranchTerminatorOpInterface>(op));

  auto *visitOp = mlir::isa<mlir::RegionBranchTerminatorOpInterface>(op)
                      ? op->getParentOp()
                      : op;

  EscapeLattice *operands[] = {getLatticeElement(operand.get())};

  llvm::SmallVector<const EscapeLattice *, 4> results;
  for (const mlir::Value result : visitOp->getResults())
    results.push_back(getLatticeElement(result));

  (void)visitOperation(visitOp, operands, results);
}

void EscapeAnalysis::visitCallOperand(mlir::OpOperand &operand) {
  mlir::Operation *op = operand.getOwner();
  assert(mlir::isa<CallOpInterface>(op));

  EscapeLattice *lattice = getLatticeElement(operand.get());
  propagateIfChanged(lattice, lattice->markEscapes());
}

void EscapeAnalysis::visitNonControlFlowArguments(
    mlir::RegionSuccessor &successor,
    llvm::ArrayRef<mlir::BlockArgument> arguments) {}

void EscapeAnalysis::setToExitState(EscapeLattice *lattice) {
  propagateIfChanged(lattice, lattice->markEscapes());
}

} // namespace bir
} // namespace belalang
