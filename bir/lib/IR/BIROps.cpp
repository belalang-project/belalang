#include "belalang/BIR/IR/BIR.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/FunctionImplementation.h"
#include "mlir/Support/LLVM.h"

#define GET_OP_CLASSES
#include "belalang/BIR/IR/BIROps.cpp.inc"


namespace belalang {
namespace bir {

// -----------------------------------------------------------------------------
// ConstantOp
// -----------------------------------------------------------------------------

LogicalResult ConstantOp::verify() {
  mlir::Type ty = getType();
  mlir::Attribute attr = getValue();

  if (isa<bir::IntType>(ty) && isa<bir::IntegerAttr>(attr)) {
    return success();
  }

  if (isa<bir::FloatType>(ty) && isa<bir::FloatAttr>(attr)) {
    return success();
  }

  if (isa<bir::StringType>(ty) && isa<bir::StringAttr>(attr)) {
    return success();
  }

  if (isa<FunctionType>(ty) && isa<bir::FnAttr>(attr)) {
    return success();
  }

  return emitOpError() << "type and attribute mismatch.";
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

mlir::Type FuncOp::getResType() {
  return getNumResults() > 0 ? getResultTypes()[0] : mlir::Type();
}

// -----------------------------------------------------------------------------
// FuncExprOp
// -----------------------------------------------------------------------------

LogicalResult FuncExprOp::verify() {
  auto &body = getBody().front();
  auto term = body.getTerminator();

  auto returnOp = dyn_cast_or_null<bir::ReturnOp>(term);
  if (!returnOp)
    return emitOpError() << "body must be terminated by a 'bir.return' op";

  auto funcTypes = getResult().getType().getResults();
  auto returnTypes = returnOp.getOperandTypes();
  if (!llvm::equal(funcTypes, returnTypes)) {
    return emitOpError() << "returned types do not match function signature types";
  }

  return success();
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

void CallOp::build(mlir::OpBuilder &builder, mlir::OperationState &state,
                   bir::FuncOp f, mlir::ValueRange operands) {
  auto callee = mlir::SymbolRefAttr::get(builder.getContext(), f.getName());
  auto resType = f.getResType();
  build(builder, state, callee, resType, operands);
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
