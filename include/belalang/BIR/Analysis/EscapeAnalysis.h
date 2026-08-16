#ifndef BELALANG_BIR_ANALYSIS_ESCAPEANALYSIS_H_
#define BELALANG_BIR_ANALYSIS_ESCAPEANALYSIS_H_

#include "mlir/Analysis/DataFlow/SparseAnalysis.h"

namespace belalang {
namespace bir {

struct EscapeLattice : mlir::dataflow::AbstractSparseLattice {
  using AbstractSparseLattice::AbstractSparseLattice;

  void print(llvm::raw_ostream &os) const override;

  mlir::ChangeResult markEscapes();

  mlir::ChangeResult
  meet(const mlir::dataflow::AbstractSparseLattice &other) override;

  bool escapes = false;
};

class EscapeAnalysis
    : public mlir::dataflow::SparseBackwardDataFlowAnalysis<EscapeLattice> {
public:
  using SparseBackwardDataFlowAnalysis::SparseBackwardDataFlowAnalysis;

  mlir::LogicalResult
  visitOperation(mlir::Operation *op, llvm::ArrayRef<EscapeLattice *> operands,
                 llvm::ArrayRef<const EscapeLattice *> results) override;

  void visitBranchOperand(mlir::OpOperand &operand) override;

  void visitCallOperand(mlir::OpOperand &operand) override;

  void visitNonControlFlowArguments(
      mlir::RegionSuccessor &successor,
      llvm::ArrayRef<mlir::BlockArgument> arguments) override;

  void setToExitState(EscapeLattice *lattice) override;
};

} // namespace bir
} // namespace belalang

#endif // BELALANG_BIR_ANALYSIS_ESCAPEANALYSIS_H_
