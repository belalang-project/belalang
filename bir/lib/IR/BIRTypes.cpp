#include "belalang/BIR/IR/BIR.h"

namespace belalang {
namespace bir {

mlir::Type StructType::parse(mlir::AsmParser &p) {
  mlir::MLIRContext *ctx = p.getContext();
  std::string name;
  llvm::SmallVector<mlir::Type> members;

  // `<`
  if (p.parseLess().failed())
    return {};

  // `<structname>`
  if (p.parseString(&name).failed())
    return {};

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

  auto nameAttr = mlir::StringAttr::get(ctx, name);
  return StructType::get(ctx, members, nameAttr);
}

void StructType::print(mlir::AsmPrinter &p) const {
  p << '<';
  p.printString(getName());
  p << ", {";
  llvm::interleaveComma(getMembers(), p);
  p << "}>";
}

} // namespace bir
} // namespace belalang
