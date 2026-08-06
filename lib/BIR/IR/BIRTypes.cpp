#include "belalang/BIR/IR/BIR.h"

namespace belalang {
namespace bir {

mlir::Type StructType::parse(mlir::AsmParser &p) {
  const mlir::SMLoc loc = p.getCurrentLocation();
  const mlir::Location eLoc = p.getEncodedSourceLoc(loc);

  mlir::MLIRContext *ctx = p.getContext();
  std::string name;
  llvm::SmallVector<mlir::Type> members;

  // `<`
  if (p.parseLess().failed())
    return {};

  // `<structname>`
  if (p.parseString(&name).failed())
    return {};

  // Matches optional `>`.
  // This parses a struct type without a body used in self-referencing types.
  SMLoc greaterLoc = p.getCurrentLocation();
  if (p.parseOptionalGreater().succeeded()) {
    auto nameAttr = p.getBuilder().getStringAttr(name);
    auto structTy = StructType::get(ctx, {}, nameAttr);

    // No struct type in the current parse stack.
    if (succeeded(p.tryStartCyclicParse(structTy))) {
      p.emitError(greaterLoc,
                  "struct without a body only allowed in a recursive struct");
      return {};
    }

    return structTy;
  }

  auto nameAttr = mlir::StringAttr::get(ctx, name);
  auto structTy = StructType::get(ctx, {}, nameAttr);

  auto cyclicParse = p.tryStartCyclicParse(structTy);
  if (failed(cyclicParse)) {
    p.emitError(loc, "nested recursive struct definition is not supported");
    return {};
  }

  // `,`
  if (p.parseComma().failed())
    return {};

  // `{<member1>, <member2>, ..., <membern>}`
  if (p.parseCommaSeparatedList(AsmParser::Delimiter::Braces, [&p, &members]() {
         return p.parseType(members.emplace_back());
       }).failed())
    return {};

  // `>`
  if (p.parseGreater().failed())
    return {};

  if (structTy.mutate(members, nameAttr).failed()) {
    p.emitError(loc, "failed to mutate recursive struct");
    return {};
  }
  return structTy;
}

void StructType::print(mlir::AsmPrinter &p) const {
  FailureOr<AsmPrinter::CyclicPrintReset> cyclicPrint;

  p << '<';
  cyclicPrint = p.tryStartCyclicPrint(*this);
  p.printString(getName());
  if (failed(cyclicPrint)) {
    p << ">";
    return;
  }
  p << ", {";
  llvm::interleaveComma(getMembers(), p);
  p << "}>";
}

mlir::LogicalResult
StructType::verify(llvm::function_ref<mlir::InFlightDiagnostic()> emitError,
                   llvm::ArrayRef<mlir::Type> members, mlir::StringAttr name) {
  if (!name)
    return emitError() << "struct type must have a name";
  return mlir::success();
}

llvm::ArrayRef<mlir::Type> StructType::getMembers() const {
  return getImpl()->members;
}
mlir::StringAttr StructType::getName() const { return getImpl()->name; }

} // namespace bir
} // namespace belalang
