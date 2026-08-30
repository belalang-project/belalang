#include "belalang/BIR/IR/BIR.h"

#include "belalang/BIR/Interfaces/LoopOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/Interfaces/FunctionImplementation.h"
#include "mlir/Interfaces/MemorySlotInterfaces.h"
#include "mlir/Support/LLVM.h"

#define GET_OP_CLASSES
#include "belalang/BIR/IR/BIROps.cpp.inc"


namespace belalang {
namespace bir {

namespace {

mlir::TypedAttr getDefaultAttr(mlir::MLIRContext *ctx, mlir::Type type) {
  if (mlir::isa<bir::IntType>(type))
    return bir::IntegerAttr::get(ctx, type, llvm::APInt(64, 0));

  if (mlir::isa<bir::FloatType>(type))
    return bir::FloatAttr::get(ctx, type, llvm::APFloat(0.0));

  if (mlir::isa<bir::StringType>(type))
    return bir::StringAttr::get(ctx, type, "");

  if (mlir::isa<bir::BoolType>(type))
    return bir::BoolAttr::get(ctx, type, false);

  if (auto structType = mlir::dyn_cast<bir::StructType>(type)) {
    llvm::SmallVector<mlir::Attribute> members;
    llvm::SmallVector<mlir::Type> memberTypes;
    for (mlir::Type memberType : structType.getMembers()) {
      mlir::TypedAttr memberAttr = getDefaultAttr(ctx, memberType);
      if (!memberAttr)
        return {};
      members.push_back(memberAttr);
      memberTypes.push_back(memberType);
    }
    return bir::StructAttr::get(ctx, type, members, memberTypes);
  }

  return {};
}

} // namespace

// -----------------------------------------------------------------------------
// ConstantOp
// -----------------------------------------------------------------------------

mlir::LogicalResult ConstantOp::verify() {
  mlir::Type ty = getType();
  mlir::Attribute attr = getValue();

  if (mlir::isa<bir::IntType>(ty) && mlir::isa<bir::IntegerAttr>(attr)) {
    return mlir::success();
  }

  if (mlir::isa<bir::FloatType>(ty) && mlir::isa<bir::FloatAttr>(attr)) {
    return mlir::success();
  }

  if (mlir::isa<bir::StringType>(ty) && mlir::isa<bir::StringAttr>(attr)) {
    return mlir::success();
  }

  if (mlir::isa<mlir::FunctionType>(ty) && mlir::isa<bir::FnAttr>(attr)) {
    return mlir::success();
  }

  if (mlir::isa<bir::BoolType>(ty) && mlir::isa<bir::BoolAttr>(attr)) {
    return mlir::success();
  }

  if (mlir::isa<bir::StructType>(ty) && mlir::isa<bir::StructAttr>(attr)) {
    return mlir::success();
  }

  if (mlir::isa<bir::ArrayType>(ty) && mlir::isa<bir::ArrayAttr>(attr)) {
    return mlir::success();
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

mlir::LogicalResult FuncOp::verify() {
  if (getFunctionType().getNumResults() > 1)
    return emitOpError() << "supports at most one result";

  return mlir::success();
}

// -----------------------------------------------------------------------------
// FuncExprOp
// -----------------------------------------------------------------------------

mlir::LogicalResult FuncExprOp::verify() {
  auto &body = getBody().front();
  auto term = body.getTerminator();

  auto returnOp = mlir::dyn_cast_or_null<bir::ReturnOp>(term);
  if (!returnOp)
    return emitOpError() << "body must be terminated by a 'bir.return' op";

  auto funcTypes = getResult().getType().getResults();
  auto returnTypes = returnOp.getOperandTypes();
  if (!llvm::equal(funcTypes, returnTypes)) {
    return emitOpError() << "returned types do not match function signature types";
  }

  return mlir::success();
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

  llvm::SmallVector<mlir::Type> argTypes;
  llvm::SmallVector<mlir::DictionaryAttr> argAttrs;
  llvm::SmallVector<mlir::Type> resultTypes;
  llvm::SmallVector<mlir::DictionaryAttr> resultAttrs;

  if (mlir::call_interface_impl::parseFunctionSignature(
          parser, argTypes, argAttrs, resultTypes, resultAttrs))
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

  mlir::call_interface_impl::printFunctionSignature(
      p, getOperands().getTypes(), getArgAttrsAttr(), false,
      (*this)->getResultTypes(), getResAttrsAttr());
}

// -----------------------------------------------------------------------------
// CallOp: SymbolUserOpInterface
// -----------------------------------------------------------------------------

mlir::LogicalResult
CallOp::verifySymbolUses(mlir::SymbolTableCollection &symbolTable) {
  auto fnAttr = (*this)->getAttrOfType<mlir::FlatSymbolRefAttr>("callee");
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

// -----------------------------------------------------------------------------
// IfOp
// -----------------------------------------------------------------------------

mlir::LogicalResult IfOp::ensureRegionTerm(mlir::Builder &b, mlir::Region &r) {
  mlir::OpBuilder builder(b.getContext());

  if (r.empty())
    builder.createBlock(&r);

  mlir::Block &blk = r.back();
  if (!blk.empty() && blk.back().hasTrait<mlir::OpTrait::IsTerminator>())
    return mlir::success();

  // Create the yield op to terminate the block
  builder.setInsertionPointToEnd(&blk);
  bir::YieldOp::create(builder, builder.getUnknownLoc());
  return mlir::success();
}

mlir::ParseResult IfOp::parse(mlir::OpAsmParser &p, mlir::OperationState &result) {
  result.regions.reserve(2);
  mlir::Region *thenRegion = result.addRegion();
  mlir::Region *elseRegion = result.addRegion();

  mlir::OpAsmParser::UnresolvedOperand cond;
  mlir::Type ty = bir::BoolType::get(p.getContext());

  if (mlir::failed(p.parseOperand(cond)) ||
      mlir::failed(p.resolveOperand(cond, ty, result.operands)))
    return mlir::failure();

  if (mlir::failed(p.parseRegion(*thenRegion)))
    return mlir::failure();
  if (mlir::failed(ensureRegionTerm(p.getBuilder(), *thenRegion)))
    return mlir::failure();

  if (mlir::succeeded(p.parseOptionalKeyword("else"))) {
    if (mlir::failed(p.parseRegion(*elseRegion)))
      return mlir::failure();
    if (mlir::failed(ensureRegionTerm(p.getBuilder(), *elseRegion)))
      return mlir::failure();
  }

  if (p.parseOptionalColon().succeeded()) {
    // The if op can only have a return type if it also has an else region.
    if (elseRegion->empty())
      return p.emitError(p.getCurrentLocation()) << "with result types should also have an else region";

    mlir::Type resultTy;
    if (p.parseType(resultTy).failed())
      return mlir::failure();
    result.addTypes(resultTy);
  }

  if (mlir::failed(p.parseOptionalAttrDict(result.attributes)))
    return mlir::failure();

  return mlir::success();
}

void IfOp::print(mlir::OpAsmPrinter &p) {
  p << ' ' << getCond() << ' ';
  p.printRegion(getThenRegion());
  
  mlir::Region &elseRegion = getElseRegion();
  if (!elseRegion.empty()) {
    p << " else ";
    p.printRegion(elseRegion);
  }

  p.printOptionalAttrDict(getOperation()->getAttrs());
}

// -----------------------------------------------------------------------------
// ConditionOp
// -----------------------------------------------------------------------------

mlir::LogicalResult bir::ConditionOp::verify() {
  if (!mlir::isa<LoopOpInterface>(getOperation()->getParentOp()))
    return emitOpError("must be within a conditional region");
  return mlir::success();
}

// -----------------------------------------------------------------------------
// AllocStackOp: PromotableAllocationOpInterface
// -----------------------------------------------------------------------------

llvm::SmallVector<mlir::MemorySlot> AllocStackOp::getPromotableSlots() {
  // Single-element stack allocation is promoted to a scalar SSA value.
  auto refType = mlir::cast<bir::RefType>(getType());
  return {mlir::MemorySlot{getResult(), refType.getReferent()}};
}

mlir::Value AllocStackOp::getDefaultValue(const mlir::MemorySlot &slot,
                                          mlir::OpBuilder &builder) {
  mlir::TypedAttr attr = getDefaultAttr(getContext(), slot.elemType);
  if (!attr)
    return {};
  return bir::ConstantOp::create(builder, getLoc(), slot.elemType, attr);
}

void AllocStackOp::handleBlockArgument(const mlir::MemorySlot &slot,
                                       mlir::BlockArgument argument,
                                       mlir::OpBuilder &builder) {}

std::optional<mlir::PromotableAllocationOpInterface>
AllocStackOp::handlePromotionComplete(const mlir::MemorySlot &slot,
                                      mlir::Value defaultValue,
                                      mlir::OpBuilder &builder) {
  if (defaultValue && defaultValue.use_empty())
    defaultValue.getDefiningOp()->erase();
  this->erase();
  return std::nullopt;
}

// -----------------------------------------------------------------------------
// LoadOp: PromotableMemOpInterface
// -----------------------------------------------------------------------------

bool LoadOp::loadsFrom(const mlir::MemorySlot &slot) {
  return getRef() == slot.ptr;
}

bool LoadOp::storesTo(const mlir::MemorySlot &slot) { return false; }

mlir::Value LoadOp::getStored(const mlir::MemorySlot &slot,
                              mlir::OpBuilder &builder, mlir::Value reachingDef,
                              const mlir::DataLayout &dataLayout) {
  llvm_unreachable("getStored should not be called on LoadOp");
}

bool LoadOp::canUsesBeRemoved(
    const mlir::MemorySlot &slot,
    const llvm::SmallPtrSetImpl<mlir::OpOperand *> &blockingUses,
    llvm::SmallVectorImpl<mlir::OpOperand *> &newBlockingUses,
    const mlir::DataLayout &dataLayout) {
  if (blockingUses.size() != 1)
    return false;

  mlir::Value blockingUse = (*blockingUses.begin())->get();
  return blockingUse == slot.ptr && getRef() == slot.ptr &&
         getResult().getType() == slot.elemType;
}

mlir::DeletionKind LoadOp::removeBlockingUses(
    const mlir::MemorySlot &slot,
    const llvm::SmallPtrSetImpl<mlir::OpOperand *> &blockingUses,
    mlir::OpBuilder &builder, mlir::Value reachingDefinition,
    const mlir::DataLayout &dataLayout) {
  getResult().replaceAllUsesWith(reachingDefinition);
  return mlir::DeletionKind::Delete;
}

// -----------------------------------------------------------------------------
// StoreOp: PromotableMemOpInterface
// -----------------------------------------------------------------------------

bool StoreOp::loadsFrom(const mlir::MemorySlot &slot) { return false; }

bool StoreOp::storesTo(const mlir::MemorySlot &slot) {
  return getDest() == slot.ptr;
}

mlir::Value StoreOp::getStored(const mlir::MemorySlot &slot,
                               mlir::OpBuilder &builder,
                               mlir::Value reachingDef,
                               const mlir::DataLayout &dataLayout) {
  return getSrc();
}

bool StoreOp::canUsesBeRemoved(
    const mlir::MemorySlot &slot,
    const llvm::SmallPtrSetImpl<mlir::OpOperand *> &blockingUses,
    llvm::SmallVectorImpl<mlir::OpOperand *> &newBlockingUses,
    const mlir::DataLayout &dataLayout) {
  if (blockingUses.size() != 1)
    return false;

  mlir::Value blockingUse = (*blockingUses.begin())->get();
  return blockingUse == slot.ptr && getDest() == slot.ptr &&
         getSrc() != slot.ptr && getSrc().getType() == slot.elemType;
}

mlir::DeletionKind StoreOp::removeBlockingUses(
    const mlir::MemorySlot &slot,
    const llvm::SmallPtrSetImpl<mlir::OpOperand *> &blockingUses,
    mlir::OpBuilder &builder, mlir::Value reachingDefinition,
    const mlir::DataLayout &dataLayout) {
  return mlir::DeletionKind::Delete;
}

} // namespace bir
} // namespace belalang
