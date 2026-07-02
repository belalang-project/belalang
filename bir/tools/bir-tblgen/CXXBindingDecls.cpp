#include "belalang/BIR/IR/BIR.h"

#include "TableGenBackends.h"
#include "Utils.h"
#include "mlir/TableGen/Operator.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/TableGen/TableGenBackend.h"

namespace {

std::vector<std::string> builderFunctionDecls;

std::string getArgs(mlir::tblgen::Operator op) {
  std::vector<std::string> args;

  for (auto operand : op.getOperands()) {
    args.push_back("const BIRValue &" + operand.name.str());
  }

  return llvm::join(args, ", ");
}

void emitGuardClassDecl(OpMetadata M, llvm::raw_ostream &os) {
  std::vector<std::string> regionNames = M.getRegionNames();

  os << "class " << M.getGuardName() << ": BIRGuard {\n";
  os << "public:\n";

  // Constructor
  os << "  " << M.getGuardName() << "(mlir::OpBuilder &builder\n";
  for (auto r : regionNames)
    os.indent(4) << ", mlir::Region *" << r << "\n";
  os.indent(2) << ") ";

  // Initializers
  os << ": BIRGuard(builder)\n";
  for (auto r : regionNames)
    os.indent(4) << ", " << r << "(" << r << ")\n";
  os.indent(2) << "{}\n";

  // Destructor
  os.indent(2) << "~" << M.getGuardName() << "() = default;\n";

  // Member Functions
  for (auto r : regionNames)
    os.indent(2) << "void enter_" << r << "();\n";

  // Data members
  os << "private:\n";
  for (auto r : regionNames)
    os.indent(2) << "mlir::Region *" << r << ";\n";

  os << "};\n";
}

void gatherBuilderFunctionDecls(OpMetadata M) {
  auto op = M.getOp();

  std::string retTy =
      M.requiresGuard() ? "std::unique_ptr<" + M.getGuardName() + ">" : "void";
  auto args = getArgs(op);

  std::string fn = "  " + retTy + " " + M.getBuilderName() + "(" + args + ");";
  builderFunctionDecls.push_back(fn);
}

void emitAndGather(const llvm::Record *opRec, llvm::raw_ostream &os) {
  if (!opRec->getValueAsBit("hasBIRGenBindings"))
    return;

  mlir::tblgen::Operator op(opRec);
  OpMetadata M(op);

  gatherBuilderFunctionDecls(M);
  if (M.requiresGuard())
    emitGuardClassDecl(M, os);
}

void emitBIRGenClass(llvm::raw_ostream &os) {
  os << "class BIRGen2 {\n"
     << "public:\n"
     << "  BIRGen2(::belalang::birgen::BIRGen &gen);\n"
     << "  ~BIRGen2() = default;\n";

  for (auto decl : builderFunctionDecls)
    os << decl << "\n";

  os << "private:\n"
     << "  ::belalang::birgen::BIRGen &gen;\n"
     << "};\n"
     << "std::unique_ptr<BIRGen2> create_birgen2(uintptr_t gen);\n";
}

} // namespace

static const char *const BIRGenForwardDecl = R"(
namespace belalang {
namespace birgen {
class BIRGen;
} // namespace birgen
} // namespace belalang
)";

static const char *const BIRGuardBaseClass = R"(
class BIRGuard {
public:
  BIRGuard(mlir::OpBuilder &builder) : guard(builder), builder(builder) {}
  virtual ~BIRGuard() = default;

protected:
  mlir::OpBuilder::InsertionGuard guard;
  mlir::OpBuilder &builder;
};
)";

namespace belalang::bir {

void emitCXXBindingDecls(const llvm::RecordKeeper &rk, llvm::raw_ostream &os) {
  os << BIRGenForwardDecl;
  os << "namespace belalang::birgen2 {\n";
  os << BIRGuardBaseClass;

  for (const auto *op : rk.getAllDerivedDefinitions("BIR_Op"))
    emitAndGather(op, os);
  emitBIRGenClass(os);

  os << "} // namespace belalang::birgen2\n";
}

} // namespace belalang::bir
