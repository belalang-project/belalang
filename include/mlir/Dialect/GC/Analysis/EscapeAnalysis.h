#ifndef MLIR_DIALECT_GC_ANALYSIS_ESCAPE_ANALYSIS_H_
#define MLIR_DIALECT_GC_ANALYSIS_ESCAPE_ANALYSIS_H_

#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace mlir {
namespace gc {

struct EscapeLattice : mlir::dataflow::AbstractSparseLattice {
  using dataflow::AbstractSparseLattice::AbstractSparseLattice;

  void print(llvm::raw_ostream &os) const override;

  mlir::ChangeResult
  meet(const mlir::dataflow::AbstractSparseLattice &other) override;

  mlir::ChangeResult markEscapes();

  bool escapes = false;
};

struct EscapeAnalysis
    : mlir::dataflow::SparseBackwardDataFlowAnalysis<EscapeLattice> {
  using SparseBackwardDataFlowAnalysis::SparseBackwardDataFlowAnalysis;

  mlir::LogicalResult
  visitOperation(mlir::Operation *op, llvm::ArrayRef<EscapeLattice *> operands,
                 llvm::ArrayRef<const EscapeLattice *> results) override;

  void visitBranchOperand(mlir::OpOperand &) override;

  void visitCallOperand(mlir::OpOperand &operand) override;

  void visitNonControlFlowArguments(
      mlir::RegionSuccessor &successor,
      llvm::ArrayRef<mlir::BlockArgument> arguments) override;

  void setToExitState(EscapeLattice *lattice) override;
};

} // namespace gc
} // namespace mlir

#endif // MLIR_DIALECT_GC_ANALYSIS_ESCAPE_ANALYSIS_H_
