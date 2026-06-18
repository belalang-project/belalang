#include "belalang/BIR/IR/BIR.h"

#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/DialectImplementation.h"

#include "llvm/ADT/APFloat.h"

static ParseResult parseIntLiteral(AsmParser &parser,
                                   llvm::APInt &value,
                                   Type type) {
  int64_t val;
  if (parser.parseInteger(val))
    return parser.emitError(parser.getCurrentLocation(), "expected integer value");

  value = llvm::APInt(64, val, true);
  return success();
}

static void printIntLiteral(AsmPrinter &p,
                            const llvm::APInt &value,
                            Type type) {
  p << value.getSExtValue();
}

static ParseResult parseFloatLiteral(AsmParser &p,
                                   mlir::FailureOr<llvm::APFloat> &value,
                                   Type type) {
  auto floatType = llvm::dyn_cast<FloatType>(type);
  if (!floatType)
    return p.emitError(p.getCurrentLocation(), "expected a floating-point type");

  llvm::APFloat floatVal(0.0);
  if (p.parseFloat(floatType.getFloatSemantics(), floatVal))
    return failure();

  value.emplace(floatVal);
  return success();
}

static void printFloatLiteral(AsmPrinter &p,
                            const llvm::APFloat &value,
                            Type type) {
  p << value.convertToDouble();
}

#define GET_TYPEDEF_CLASSES
#include "belalang/BIR/IR/BIRTypes.cpp.inc"

#define GET_ATTRDEF_CLASSES
#include "belalang/BIR/IR/BIRAttrs.cpp.inc"

#include "belalang/BIR/IR/BIRDialect.cpp.inc"

namespace belalang {
namespace bir {

void BIRDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "belalang/BIR/IR/BIRTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "belalang/BIR/IR/BIROps.cpp.inc"
      >();

  addAttributes<
#define GET_ATTRDEF_LIST
#include "belalang/BIR/IR/BIRAttrs.cpp.inc"
      >();
}

} // namespace bir
} // namespace belalang
