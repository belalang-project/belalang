#include "TableGenBackends.h"
#include "Utils.h"
#include "mlir/TableGen/Operator.h"
#include "llvm/TableGen/TableGenBackend.h"

namespace {

std::string getArgs(mlir::tblgen::Operator op) {
  std::vector<std::string> args;

  for (auto operand : op.getOperands()) {
    args.push_back("const BIRValue &" + operand.name.str());
  }

  return llvm::join(args, ", ");
}

void emitGuardMemberFunctionDefs(OpMetadata M, llvm::raw_ostream &os) {
  for (auto r : M.getRegionNames()) {
    os << "void " + M.getGuardName() + "::enter_" + r + "() {\n";
    os.indent(2) << r + "->push_back(new mlir::Block());\n";
    os.indent(2) << "builder.setInsertionPointToEnd(&" + r + "->front());\n";
    os << "}\n\n";
  }
}

void emitBuilderFunctionDef(OpMetadata M, llvm::raw_ostream &os) {
  std::string retTy =
      M.requiresGuard() ? "std::unique_ptr<" + M.getGuardName() + ">" : "void";

  auto op = M.getOp();
  auto args = getArgs(op);

  os << retTy + " BIRGen::" + M.getBuilderName() + "(" + args + ") {\n";

  if (op.getNumResults() > 0)
    os.indent(2) << "return nullptr;\n";
  else if (M.requiresGuard()) {
    auto regionGetters =
        llvm::join(llvm::map_range(M.getRegionNames(),
                                   [&](const std::string &s) {
                                     auto fnName = sc2cc("get_" + s);
                                     return "&op." + fnName + "()";
                                   }),
                   ", ");
    os.indent(2) << "auto op = " + M.getFullOpIdent() +
                        "::create(gen.builder, gen.loc);\n";
    os.indent(2) << "return std::make_unique<" + M.getGuardName() +
                        ">(gen.builder, " + regionGetters + ");\n";
  } else if (op.getNumResults() == 0 && op.getNumOperands() == 0)
    os.indent(2) << M.getFullOpIdent() + "::create(gen.builder, gen.loc);\n";

  os << "}\n\n";
}

void emit(const llvm::Record *opRec, llvm::raw_ostream &os) {
  if (!opRec->getValueAsBit("hasBIRGenBindings"))
    return;

  mlir::tblgen::Operator op(opRec);
  OpMetadata M(op);

  emitCommentBanner(os, M.getOpIdent());
  emitBuilderFunctionDef(M, os);
  if (M.requiresGuard())
    emitGuardMemberFunctionDefs(M, os);
}

} // namespace

namespace belalang::bir {

void emitCXXBindingDefs(const llvm::RecordKeeper &rk, llvm::raw_ostream &os) {
  for (const auto *op : rk.getAllDerivedDefinitions("BIR_Op"))
    emit(op, os);
}

} // namespace belalang::bir
