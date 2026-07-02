#include "belalang/BIR/IR/BIR.h"

#include "TableGenBackends.h"
#include "Utils.h"
#include "mlir/TableGen/Operator.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/TableGen/TableGenBackend.h"

namespace {

std::vector<std::string> builderFunctions;
std::vector<std::string> guards;

const char *const BIRGenClass = R"(
class BIRGen2 {{
public:
  BIRGen2(::belalang::birgen::BIRGen &gen);
  ~BIRGen2() = default;

{0}

private:
  ::belalang::birgen::BIRGen &gen;
};

std::unique_ptr<BIRGen2> create_birgen2(uintptr_t gen);
)";

const char *const BIRGuardClass = R"(
class BIRGuard {
public:
  BIRGuard(mlir::OpBuilder &builder) : guard(builder), builder(builder) {}
  virtual ~BIRGuard() = default;

protected:
  mlir::OpBuilder::InsertionGuard guard;
  mlir::OpBuilder &builder;
};
)";

// 0: op name
// 1: region members
// 2: member functions to enter regions
// 3: initializers
// 4: args
const char *const BIRGuardChildClass = R"(
class BIR{0}Guard : BIRGuard {
public:
  BIR{0}Guard(mlir::OpBuilder &builder, {4})
      : BIRGuard(builder), {3} {{}
  ~BIR{0}Guard() = default;

{2}

private:
{1}
};
)";

std::string getArgs(mlir::tblgen::Operator op) {
  std::vector<std::string> args;

  for (auto operand : op.getOperands()) {
    args.push_back("const BIRValue &" + operand.name.str());
  }

  return llvm::join(args, ", ");
}

void genGuard(OpMetadata M) {
  auto op = M.getOp();
  llvm::StringRef name = op.getCppClassName();

  std::vector<std::string> regionNames = M.getRegionNames();

  std::string dataMembers =
      llvm::join(llvm::map_range(regionNames,
                                 [&](const std::string &s) {
                                   return "  mlir::Region *" + s + ";";
                                 }),
                 "\n");

  std::string memberFunctions =
      llvm::join(llvm::map_range(regionNames,
                                 [&](const std::string &s) {
                                   return "  void enter_" + s + "();";
                                 }),
                 "\n");

  std::string inits = llvm::join(
      llvm::map_range(regionNames,
                      [&](const std::string &s) { return s + "(" + s + ")"; }),
      ", ");

  std::string args = llvm::join(llvm::map_range(regionNames,
                                                [&](const std::string &s) {
                                                  return "mlir::Region *" + s;
                                                }),
                                ", ");

  guards.push_back(llvm::formatv(BIRGuardChildClass, name, dataMembers,
                                 memberFunctions, inits, args));
}

void genBuilderFunctionDecl(const llvm::Record *opRec, llvm::raw_ostream &os) {
  mlir::tblgen::Operator op(opRec);
  OpMetadata M(op);

  if (!opRec->getValueAsBit("hasBIRGenBindings"))
    return;

  std::string retTy =
      M.requiresGuard() ? "std::unique_ptr<" + M.getGuardName() + ">" : "void";
  auto args = getArgs(op);

  std::string fn = "  " + retTy + " " + M.getBuilderName() + "(" + args + ");";
  builderFunctions.push_back(fn);

  if (M.requiresGuard())
    genGuard(M);
}

} // namespace

namespace belalang::bir {

void emitCXXBindingDecls(const llvm::RecordKeeper &rk, llvm::raw_ostream &os) {
  for (const auto *op : rk.getAllDerivedDefinitions("BIR_Op"))
    genBuilderFunctionDecl(op, os);

  os << "namespace belalang {\n"
     << "namespace birgen {\n"
     << "class BIRGen;\n"
     << "} // namespace birgen\n"
     << "} // namespace belalang\n"
     << "namespace belalang::birgen2 {\n";

  os << BIRGuardClass << "\n";
  os << llvm::join(guards, "\n");
  os << llvm::formatv(BIRGenClass, llvm::join(builderFunctions, "\n"));

  os << "} // namespace belalang::birgen2\n";
}

} // namespace belalang::bir
