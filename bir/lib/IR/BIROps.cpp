#include "belalang/BIR/IR/BIR.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/FunctionImplementation.h"
#include "mlir/Support/LLVM.h"

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

// -----------------------------------------------------------------------------
// FuncOp
// -----------------------------------------------------------------------------

void FuncOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                   llvm::StringRef name, mlir::FunctionType type,
                   llvm::ArrayRef<mlir::NamedAttribute> attrs) {
  state.addRegion();
  state.addAttribute(mlir::SymbolTable::getSymbolAttrName(),
                     builder.getStringAttr(name));
  state.addAttribute(getFunctionTypeAttrName(state.name),
                     mlir::TypeAttr::get(type));
  state.attributes.append(attrs.begin(), attrs.end());
}

mlir::ParseResult FuncOp::parse(mlir::OpAsmParser &parser,
                                mlir::OperationState &result) {
  auto buildFuncType =
      [](mlir::Builder &builder, llvm::ArrayRef<mlir::Type> argTypes,
         llvm::ArrayRef<mlir::Type> results,
         mlir::function_interface_impl::VariadicFlag,
         std::string &) { return builder.getFunctionType(argTypes, results); };

  return mlir::function_interface_impl::parseFunctionOp(
      parser, result, false, getFunctionTypeAttrName(result.name),
      buildFuncType, getArgAttrsAttrName(result.name),
      getResAttrsAttrName(result.name));
}

void FuncOp::print(mlir::OpAsmPrinter &p) {
  mlir::function_interface_impl::printFunctionOp(
      p, *this, false, getFunctionTypeAttrName(), getArgAttrsAttrName(),
      getResAttrsAttrName());
}

// -----------------------------------------------------------------------------
// CallOp
// -----------------------------------------------------------------------------

void CallOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                   mlir::SymbolRefAttr callee, mlir::Type resType,
                   mlir::ValueRange operands) {
  state.addOperands(operands);
  if (callee)
    state.addAttribute("callee", callee);
  if (resType)
    state.addTypes(resType);
}

mlir::ParseResult CallOp::parse(mlir::OpAsmParser &parser,
                                mlir::OperationState &result) {
  mlir::FlatSymbolRefAttr calleeAttr;
  llvm::SMLoc opsLoc;
  llvm::SmallVector<mlir::OpAsmParser::UnresolvedOperand, 4> ops;

  if (parser.parseAttribute(calleeAttr, "callee", result.attributes))
    return mlir::failure();

  opsLoc = parser.getCurrentLocation();
  if (parser.parseOperandList(ops, mlir::AsmParser::Delimiter::Paren))
    return mlir::failure();

  if (parser.parseOptionalAttrDict(result.attributes))
    return mlir::failure();

  if (parser.parseColon())
    return mlir::failure();

  SmallVector<Type> argTypes;
  SmallVector<DictionaryAttr> argAttrs;
  SmallVector<Type> resultTypes;
  SmallVector<DictionaryAttr> resultAttrs;

  if (call_interface_impl::parseFunctionSignature(parser, argTypes, argAttrs,
                                                  resultTypes, resultAttrs))
    return mlir::failure();

  result.addTypes(resultTypes);

  if (parser.resolveOperands(ops, argTypes, opsLoc, result.operands))
    return mlir::failure();

  return mlir::success();
}

void CallOp::print(mlir::OpAsmPrinter &p) {
  p << ' ';
  p.printAttributeWithoutType(getCalleeAttr());
  p << '(' << getArgOperands() << ')';
  p << " : ";

  call_interface_impl::printFunctionSignature(
      p, getOperands().getTypes(), getArgAttrsAttr(), false,
      (*this)->getResultTypes(), getResAttrsAttr());
}

// -----------------------------------------------------------------------------
// CallOp: SymbolUserOpInterface
// -----------------------------------------------------------------------------

mlir::LogicalResult
CallOp::verifySymbolUses(mlir::SymbolTableCollection &symbolTable) {
  auto fnAttr = (*this)->getAttrOfType<FlatSymbolRefAttr>("callee");
  auto fn = symbolTable.lookupNearestSymbolFrom<bir::FuncOp>(*this, fnAttr);

  if (!fn)
    return (*this)->emitOpError() << "'" << fnAttr.getValue()
                                  << "' does not reference a valid function";

  return mlir::success();
}

// -----------------------------------------------------------------------------
// CallOp: CallOpInterface
// -----------------------------------------------------------------------------

mlir::OperandRange CallOp::getArgOperands() { return getArgs(); }
mlir::MutableOperandRange CallOp::getArgOperandsMutable() {
  return getArgsMutable();
}

mlir::CallInterfaceCallable CallOp::getCallableForCallee() {
  return (*this)->getAttrOfType<mlir::SymbolRefAttr>("callee");
}

void CallOp::setCalleeFromCallable(mlir::CallInterfaceCallable callee) {
  (*this)->setAttr("callee", mlir::cast<mlir::SymbolRefAttr>(callee));
}

} // namespace bir
} // namespace belalang
