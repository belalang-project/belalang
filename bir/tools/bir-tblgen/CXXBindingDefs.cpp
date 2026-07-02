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

void genGuardMemberFunctions(OpMetadata M, llvm::raw_ostream &os) {
  for (auto r : M.getRegionNames()) {
    os << "void " + M.getGuardName() + "::enter_" + r + "() {\n";
    os.indent(2) << r + "->push_back(new mlir::Block());\n";
    os.indent(2) << "builder.setInsertionPointToEnd(&" + r + "->front());\n";
    os << "}\n\n";
  }
}

void genBuilderFunctionDefs(const llvm::Record *opRec, llvm::raw_ostream &os) {
  mlir::tblgen::Operator op(opRec);
  OpMetadata M(op);

  if (!opRec->getValueAsBit("hasBIRGenBindings"))
    return;

  llvm::StringRef name = op.getCppClassName();

  std::string retTy =
      M.requiresGuard() ? "std::unique_ptr<" + M.getGuardName() + ">" : "void";
  auto args = getArgs(op);

  os << retTy + " BIRGen2::build" + name.str() + "(" + args + ") {\n";

  if (op.getNumResults() > 0)
    os.indent(2) << "return nullptr;\n";
  else if (M.requiresGuard()) {
    auto regionGetters = llvm::join(
        llvm::map_range(M.getRegionNames(),
                        [&](const std::string &s) {
                          auto fnName =
                              llvm::convertToCamelFromSnakeCase("get_" + s);
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

  if (M.requiresGuard())
    genGuardMemberFunctions(M, os);
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
