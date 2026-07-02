#include "belalang/BIR/IR/BIR.h"

#include "TableGenBackends.h"
#include "mlir/TableGen/Operator.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/TableGen/TableGenBackend.h"

namespace {

std::vector<std::string> builderFunctions;

const char *const BIRGenClass = R"(
class BIRGen2 {
public:
  BIRGen2();
  ~BIRGen2() = default;

{0}

private:
  mlir::MLIRContext context;
  mlir::ModuleOp module;
  mlir::OpBuilder builder;
  mlir::Location loc;
};
)";

std::string getArgs(mlir::tblgen::Operator op) {
  std::vector<std::string> args;

  for (auto operand : op.getOperands()) {
    args.push_back("const BIRValue &" + operand.name.str());
  }

  return llvm::join(args, ", ");
}

void genBuilderFunctionDecl(const llvm::Record *opRec, llvm::raw_ostream &os) {
  mlir::tblgen::Operator op(opRec);

  // TODO: support more
  if (!(op.getNumResults() == 0 && op.getNumOperands() == 0))
    return;

  llvm::StringRef name = op.getCppClassName();

  std::string retTy =
      op.getNumResults() > 0 ? "std::unique_ptr<BIRValue>" : "void";
  auto args = getArgs(op);

  std::string fn = "  " + retTy + " build" + name.str() + "(" + args + ");";
  builderFunctions.push_back(fn);
}

void genBuilderFunctionDefs(const llvm::Record *opRec, llvm::raw_ostream &os) {
  mlir::tblgen::Operator op(opRec);

  // TODO: support more
  if (!(op.getNumResults() == 0 && op.getNumOperands() == 0))
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
                        "::create(builder, loc);\n";
  }

  os << "}\n\n";
}

} // namespace

namespace belalang::bir {

void emitCXXBindingsDecl(const llvm::RecordKeeper &rk, llvm::raw_ostream &os) {
  os << "#include \"mlir/IR/MLIRContext.h\"\n";
  os << "#include \"mlir/IR/BuiltinOps.h\"\n";
  os << "#include \"mlir/IR/Builders.h\"\n";
  os << "#include \"mlir/IR/Location.h\"\n\n";
  os << "namespace belalang::birgen2 {\n";
  for (const auto *op : rk.getAllDerivedDefinitions("BIR_Op"))
    genBuilderFunctionDecl(op, os);
  os << llvm::formatv(BIRGenClass, llvm::join(builderFunctions, "\n"));
  os << "} // namespace belalang::birgen2\n";
}

void emitCXXBindingsDefs(const llvm::RecordKeeper &rk, llvm::raw_ostream &os) {
  os << "namespace belalang::birgen2 {\n";
  for (const auto *op : rk.getAllDerivedDefinitions("BIR_Op"))
    genBuilderFunctionDefs(op, os);
  os << "} // namespace belalang::birgen2\n";
}

} // namespace belalang::bir
