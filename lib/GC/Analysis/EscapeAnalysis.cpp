#include "mlir/Dialect/GC/Analysis/EscapeAnalysis.h"
#include "mlir/Interfaces/ControlFlowInterfaces.h"

namespace mlir {
namespace gc {

// -----------------------------------------------------------------------------
// Escape Lattice
// -----------------------------------------------------------------------------

void EscapeLattice::print(llvm::raw_ostream &os) const {
  os << (escapes ? "escapes" : "noescape");
}

mlir::ChangeResult
EscapeLattice::meet(const dataflow::AbstractSparseLattice &other) {
  const auto &lattice = static_cast<const EscapeLattice &>(other);
  return lattice.escapes ? markEscapes() : ChangeResult::NoChange;
}

mlir::ChangeResult EscapeLattice::markEscapes() {
  if (escapes)
    return ChangeResult::NoChange;
  escapes = true;
  return ChangeResult::Change;
}

// -----------------------------------------------------------------------------
// Escape Analysis
// -----------------------------------------------------------------------------

mlir::LogicalResult
EscapeAnalysis::visitOperation(mlir::Operation *op,
                               llvm::ArrayRef<EscapeLattice *> operands,
                               llvm::ArrayRef<const EscapeLattice *> results) {
  if (op->hasTrait<OpTrait::ReturnLike>()) {
    for (EscapeLattice *operand : operands) {
      propagateIfChanged(operand, operand->markEscapes());
    }
  }

  // TODO: Check for more traits and ops.

  return mlir::success();
}

void EscapeAnalysis::visitBranchOperand(mlir::OpOperand &operand) {
  mlir::Operation *op = operand.getOwner();
  assert(mlir::isa<mlir::RegionBranchOpInterface>(op) ||
         mlir::isa<mlir::BranchOpInterface>(op) ||
         mlir::isa<mlir::RegionBranchTerminatorOpInterface>(op));

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
  assert(mlir::isa<mlir::CallOpInterface>(op));

  EscapeLattice *lattice = getLatticeElement(operand.get());
  propagateIfChanged(lattice, lattice->markEscapes());
}

void EscapeAnalysis::visitNonControlFlowArguments(
    mlir::RegionSuccessor &successor,
    llvm::ArrayRef<mlir::BlockArgument> arguments) {}

void EscapeAnalysis::setToExitState(EscapeLattice *lattice) {
  propagateIfChanged(lattice, lattice->markEscapes());
}

} // namespace gc
} // namespace mlir
