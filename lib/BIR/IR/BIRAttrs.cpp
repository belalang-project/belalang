#include "belalang/BIR/IR/BIR.h"

#include "mlir/IR/DialectImplementation.h"

namespace belalang {
namespace bir {

mlir::Attribute IntegerAttr::parse(mlir::AsmParser &p, mlir::Type attrType) {
  if (p.parseLess())
    return {};

  uint64_t iVal;
  if (p.parseInteger(iVal))
    return {};
  llvm::APInt value(64, iVal);

  if (p.parseGreater())
    return {};

  return IntegerAttr::get(p.getContext(), attrType, value);
}

void IntegerAttr::print(mlir::AsmPrinter &p) const {
  p << "<";
  getValue().print(p.getStream(), true);
  p << ">";
}

mlir::Attribute FloatAttr::parse(mlir::AsmParser &p, mlir::Type attrType) {
  if (p.parseLess())
    return {};

  double fVal;
  if (p.parseFloat(fVal))
    return {};
  llvm::APFloat value(fVal);

  if (p.parseGreater())
    return {};

  return FloatAttr::get(p.getContext(), attrType, value);
}

void FloatAttr::print(mlir::AsmPrinter &p) const {
  p << "<";
  p.printFloat(getValue());
  p << ">";
}

mlir::Attribute StringAttr::parse(mlir::AsmParser &p, mlir::Type attrType) {
  if (p.parseLess())
    return {};

  std::string sVal;
  if (p.parseString(&sVal))
    return {};
  llvm::StringRef value(sVal);

  if (p.parseGreater())
    return {};

  return StringAttr::get(p.getContext(), attrType, value);
}

void StringAttr::print(mlir::AsmPrinter &p) const {
  p << "<";
  p.printString(getValue());
  p << ">";
}

mlir::Attribute StructAttr::parse(mlir::AsmParser &p, mlir::Type attrType) {
  if (p.parseLess().failed())
    return {};

  llvm::SmallVector<mlir::Attribute> members;
  llvm::SmallVector<mlir::Type> memberTypes;

  auto result =
      p.parseCommaSeparatedList(mlir::AsmParser::Delimiter::Braces, [&]() {
        mlir::Attribute member;
        if (p.parseAttribute(member).failed())
          return mlir::failure();

        auto typedMember = llvm::dyn_cast<mlir::TypedAttr>(member);
        if (!typedMember)
          return mlir::failure();

        members.push_back(member);
        memberTypes.push_back(typedMember.getType());
        return mlir::success();
      });

  if (result.failed())
    return {};

  if (p.parseGreater().failed())
    return {};

  return StructAttr::get(p.getContext(), attrType, members, memberTypes);
}

void StructAttr::print(mlir::AsmPrinter &p) const {
  p << "<{";
  llvm::interleaveComma(llvm::seq<size_t>(0, getMembers().size()), p,
                        [&](size_t i) {
                          p.printAttributeWithoutType(getMembers()[i]);
                          p << " : ";
                          p.printType(getMemberTypes()[i]);
                        });
  p << "}>";
}

} // namespace bir
} // namespace belalang
