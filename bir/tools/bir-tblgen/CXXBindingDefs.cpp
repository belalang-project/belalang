#include "TableGenBackends.h"
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

void genBuilderFunctionDefs(const llvm::Record *opRec, llvm::raw_ostream &os) {
  mlir::tblgen::Operator op(opRec);

  // TODO: support more
  if (!(op.getNumResults() == 0 && op.getNumOperands() == 0 &&
        op.getNumAttributes() == 0 && op.getNumRegions() == 0))
    return;

  llvm::StringRef name = op.getCppClassName();

  std::string retTy =
      op.getNumResults() > 0 ? "std::unique_ptr<BIRValue>" : "void";
  auto args = getArgs(op);

  os << retTy + " BIRGen2::build" + name.str() + "(" + args + ") {\n";

  if (op.getNumResults() > 0)
    os.indent(2) << "return nullptr;\n";
  else if (op.getNumResults() == 0 && op.getNumOperands() == 0) {
    os.indent(2) << op.getCppNamespace() + "::" + op.getCppClassName() +
                        "::create(gen.builder, gen.loc);\n";
  }

  os << "}\n\n";
}

} // namespace

namespace belalang::bir {

void emitCXXBindingDefs(const llvm::RecordKeeper &rk, llvm::raw_ostream &os) {
  os << "namespace belalang::birgen2 {\n";
  os << "std::unique_ptr<BIRGen2> create_birgen2(uintptr_t gen_ptr) {\n";
  os << "  auto *gen = reinterpret_cast<::belalang::birgen::BIRGen "
        "*>(gen_ptr);\n";
  os << "  return std::make_unique<BIRGen2>(*gen);\n";
  os << "}\n";
  os << "BIRGen2::BIRGen2(::belalang::birgen::BIRGen &gen) : gen(gen) {}\n";
  for (const auto *op : rk.getAllDerivedDefinitions("BIR_Op"))
    genBuilderFunctionDefs(op, os);
  os << "} // namespace belalang::birgen2\n";
}

} // namespace belalang::bir
