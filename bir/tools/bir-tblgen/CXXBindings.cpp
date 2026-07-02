#include "belalang/BIR/IR/BIR.h"

#include "TableGenBackends.h"
#include "mlir/TableGen/Operator.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/TableGen/TableGenBackend.h"

namespace {

std::vector<std::string> builderFunctions;

const char *const BIRGenClass = R"(
namespace belalang {{
namespace birgen {{
class BIRGen;
} // namespace birgen
} // namespace belalang

namespace belalang::birgen2 {{

class BIRGen2 {{
public:
  BIRGen2(::belalang::birgen::BIRGen &gen);
  ~BIRGen2() = default;

{0}

private:
  ::belalang::birgen::BIRGen &gen;
};

std::unique_ptr<BIRGen2> create_birgen2(uintptr_t gen);

} // namespace belalang::birgen2
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
  if (!(op.getNumResults() == 0 && op.getNumOperands() == 0 &&
        op.getNumAttributes() == 0 && op.getNumRegions() == 0))
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

void emitCXXBindingsDecl(const llvm::RecordKeeper &rk, llvm::raw_ostream &os) {
  os << "#pragma once\n\n";
  os << "#include <cstdint>\n";
  os << "#include <memory>\n";
  os << "#include \"mlir/IR/MLIRContext.h\"\n";
  os << "#include \"mlir/IR/BuiltinOps.h\"\n";
  os << "#include \"mlir/IR/Builders.h\"\n";
  os << "#include \"mlir/IR/Location.h\"\n\n";
  for (const auto *op : rk.getAllDerivedDefinitions("BIR_Op"))
    genBuilderFunctionDecl(op, os);
  os << llvm::formatv(BIRGenClass, llvm::join(builderFunctions, "\n"));
}

void emitCXXBindingsDefs(const llvm::RecordKeeper &rk, llvm::raw_ostream &os) {
  os << "#include <memory>\n";
  os << "#include <cstdint>\n";
  os << "#include \"bindings.h\"\n";
  os << "#include \"belalang/BIR/IR/BIR.h\"\n";
  os << "#include \"belalang/BIRGen/BIRGen.h\"\n\n";
  os << "namespace belalang::birgen2 {\n";
  os << "std::unique_ptr<BIRGen2> create_birgen2(uintptr_t gen_ptr) {\n";
  os << "  auto *gen = reinterpret_cast<::belalang::birgen::BIRGen *>(gen_ptr);\n";
  os << "  return std::make_unique<BIRGen2>(*gen);\n";
  os << "}\n";
  os << "BIRGen2::BIRGen2(::belalang::birgen::BIRGen &gen) : gen(gen) {}\n";
  for (const auto *op : rk.getAllDerivedDefinitions("BIR_Op"))
    genBuilderFunctionDefs(op, os);
  os << "} // namespace belalang::birgen2\n";
}

} // namespace belalang::bir
