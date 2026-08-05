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

} // namespace bir
} // namespace belalang
