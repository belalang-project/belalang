#include "belalang/BIR/IR/BIR.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Support/LLVM.h"
#include "llvm/ADT/TypeSwitch.h"

#define GET_OP_CLASSES
#include "belalang/BIR/IR/BIROps.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "belalang/BIR/IR/BIRTypes.cpp.inc"

namespace belalang {
namespace bir {

// -----------------------------------------------------------------------------
// ConstantOp
// -----------------------------------------------------------------------------

mlir::ParseResult ConstantOp::parse(mlir::OpAsmParser &parser,
                                    mlir::OperationState &result) {
  int64_t iVal;
  if (auto res = parser.parseOptionalInteger(iVal); res.has_value()) {
    if (mlir::failed(*res))
      return mlir::failure();
    result.addAttribute("value", parser.getBuilder().getI32IntegerAttr(iVal));
  } else {
    double fVal;
    if (mlir::succeeded(parser.parseFloat(fVal))) {
      result.addAttribute("value", parser.getBuilder().getF32FloatAttr(fVal));
    } else {
      mlir::Attribute valueAttr;
      if (parser.parseAttribute(valueAttr, "value", result.attributes))
        return mlir::failure();
    }
  }

  mlir::Type type;
  if (parser.parseColonType(type))
    return mlir::failure();

  return parser.addTypeToList(type, result.types);
}

void ConstantOp::print(mlir::OpAsmPrinter &p) {
  p << " ";
  auto value = getValue();
  if (auto intAttr = mlir::dyn_cast<mlir::IntegerAttr>(value)) {
    p << intAttr.getInt();
  } else if (auto floatAttr = mlir::dyn_cast<mlir::FloatAttr>(value)) {
    p << floatAttr.getValueAsDouble();
  } else {
    p << value;
  }
  p << " : " << getType();
  p.printOptionalAttrDict((*this)->getAttrs(), {"value"});
}

} // namespace bir
} // namespace belalang
