#include "belalang/BIR/IR/BIR.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "llvm/ADT/SmallBitVector.h"
#include <numeric>

namespace belalang {
namespace bir {

// -----------------------------------------------------------------------------
// IntType
// -----------------------------------------------------------------------------

llvm::TypeSize
IntType::getTypeSizeInBits(const mlir::DataLayout &dl,
                           mlir::DataLayoutEntryListRef params) const {
  return llvm::TypeSize::getFixed(64);
}

uint64_t IntType::getABIAlignment(const mlir::DataLayout &dl,
                                  mlir::DataLayoutEntryListRef params) const {
  return 8;
}

// -----------------------------------------------------------------------------
// FloatType
// -----------------------------------------------------------------------------

llvm::TypeSize
FloatType::getTypeSizeInBits(const mlir::DataLayout &dl,
                             mlir::DataLayoutEntryListRef params) const {
  return llvm::TypeSize::getFixed(64);
}

uint64_t FloatType::getABIAlignment(const mlir::DataLayout &dl,
                                    mlir::DataLayoutEntryListRef params) const {
  return 8;
}

// -----------------------------------------------------------------------------
// BoolType
// -----------------------------------------------------------------------------

llvm::TypeSize
BoolType::getTypeSizeInBits(const mlir::DataLayout &dl,
                            mlir::DataLayoutEntryListRef params) const {
  return llvm::TypeSize::getFixed(8);
}

uint64_t BoolType::getABIAlignment(const mlir::DataLayout &dl,
                                   mlir::DataLayoutEntryListRef params) const {
  return 1;
}

// -----------------------------------------------------------------------------
// StringType
// -----------------------------------------------------------------------------

static mlir::Type getStringLLVMType(mlir::MLIRContext *ctx) {
  auto ptrType = mlir::LLVM::LLVMPointerType::get(ctx);
  auto lengthType = mlir::IntegerType::get(ctx, 64);
  return mlir::LLVM::LLVMStructType::getLiteral(ctx, {ptrType, lengthType});
}

llvm::TypeSize
StringType::getTypeSizeInBits(const mlir::DataLayout &dl,
                              mlir::DataLayoutEntryListRef params) const {
  return dl.getTypeSizeInBits(getStringLLVMType(getContext()));
}

uint64_t
StringType::getABIAlignment(const mlir::DataLayout &dl,
                            mlir::DataLayoutEntryListRef params) const {
  return dl.getTypeABIAlignment(getStringLLVMType(getContext()));
}

// -----------------------------------------------------------------------------
// StructType
// -----------------------------------------------------------------------------

llvm::TypeSize
RefType::getTypeSizeInBits(const mlir::DataLayout &dl,
                           mlir::DataLayoutEntryListRef params) const {
  auto ptrType = mlir::LLVM::LLVMPointerType::get(getContext());
  return dl.getTypeSizeInBits(ptrType);
}

uint64_t RefType::getABIAlignment(const mlir::DataLayout &dl,
                                  mlir::DataLayoutEntryListRef params) const {
  auto ptrType = mlir::LLVM::LLVMPointerType::get(getContext());
  return dl.getTypeABIAlignment(ptrType);
}

mlir::Type StructType::parse(mlir::AsmParser &p) {
  const mlir::SMLoc loc = p.getCurrentLocation();
  const mlir::Location eLoc = p.getEncodedSourceLoc(loc);

  mlir::MLIRContext *ctx = p.getContext();
  std::string name;
  llvm::SmallVector<mlir::Type> members;
  llvm::SmallVector<int32_t> reorder;

  // `<`
  if (p.parseLess().failed())
    return {};

  // `<structname>`
  if (p.parseString(&name).failed())
    return {};

  // Matches optional `>`.
  // This parses a struct type without a body used in self-referencing types.
  mlir::SMLoc greaterLoc = p.getCurrentLocation();
  if (p.parseOptionalGreater().succeeded()) {
    auto nameAttr = p.getBuilder().getStringAttr(name);
    auto structTy = StructType::get(ctx, {}, nameAttr, {});

    // No struct type in the current parse stack.
    if (succeeded(p.tryStartCyclicParse(structTy))) {
      p.emitError(greaterLoc,
                  "struct without a body only allowed in a recursive struct");
      return {};
    }

    return structTy;
  }

  auto nameAttr = mlir::StringAttr::get(ctx, name);
  auto structTy = StructType::get(ctx, {}, nameAttr, {});

  auto cyclicParse = p.tryStartCyclicParse(structTy);
  if (failed(cyclicParse)) {
    p.emitError(loc, "nested recursive struct definition is not supported");
    return {};
  }

  // `,`
  if (p.parseComma().failed())
    return {};

  // `{<member1>, <member2>, ..., <membern>}`
  if (p.parseCommaSeparatedList(
           mlir::AsmParser::Delimiter::Braces,
           [&p, &members]() { return p.parseType(members.emplace_back()); })
          .failed())
    return {};

  if (p.parseOptionalComma().succeeded()) {
    if (p.parseKeyword("reorder").failed())
      return {};
    if (p.parseEqual().failed())
      return {};
    auto parseResult = p.parseCommaSeparatedList(
        mlir::AsmParser::Delimiter::Square, [&p, &reorder]() {
          int64_t value;
          if (p.parseInteger(value).failed())
            return mlir::failure();
          reorder.push_back(static_cast<int32_t>(value));
          return mlir::success();
        });
    if (parseResult.failed())
      return {};
  }

  // `>`
  if (p.parseGreater().failed())
    return {};

  if (structTy.mutate(members, nameAttr, reorder).failed()) {
    p.emitError(loc, "failed to mutate recursive struct");
    return {};
  }
  return structTy;
}

void StructType::print(mlir::AsmPrinter &p) const {
  mlir::FailureOr<mlir::AsmPrinter::CyclicPrintReset> cyclicPrint;

  // Start.
  p << '<';

  // Struct name.
  cyclicPrint = p.tryStartCyclicPrint(*this);
  p.printString(getName());
  if (failed(cyclicPrint)) {
    p << ">";
    return;
  }

  // Struct members.
  p << ", {";
  llvm::interleaveComma(getMembers(), p);
  p << "}";

  // Struct reorder data.
  if (!getReorder().empty()) {
    p << ", reorder = [";
    llvm::interleaveComma(getReorder(), p);
    p << "]";
  }

  // End.
  p << ">";
}

mlir::LogicalResult
StructType::verify(llvm::function_ref<mlir::InFlightDiagnostic()> emitError,
                   llvm::ArrayRef<mlir::Type> members, mlir::StringAttr name,
                   llvm::ArrayRef<int32_t> reorder) {
  if (!name)
    return emitError() << "struct type must have a name";
  if (!reorder.empty() && reorder.size() != members.size())
    return emitError() << "struct reorder mapping must match member count";
  llvm::SmallBitVector seen(reorder.size());
  for (int32_t physicalIndex : reorder) {
    if (physicalIndex < 0 ||
        static_cast<size_t>(physicalIndex) >= reorder.size() ||
        seen[physicalIndex])
      return emitError() << "struct reorder mapping must be a permutation";
    seen[physicalIndex] = true;
  }
  return mlir::success();
}

llvm::ArrayRef<mlir::Type> StructType::getMembers() const {
  return getImpl()->members;
}
mlir::StringAttr StructType::getName() const { return getImpl()->name; }
llvm::ArrayRef<int32_t> StructType::getReorder() const {
  return getImpl()->reorder;
}
mlir::LogicalResult StructType::setReorder(llvm::ArrayRef<int32_t> reorder) {
  return mutate(getMembers(), getName(), reorder);
}

llvm::SmallVector<int32_t> StructType::getInverseReorder() const {
  llvm::SmallVector<int32_t> invReorder(getMembers().size());
  if (getReorder().empty()) {
    std::iota(invReorder.begin(), invReorder.end(), 0);
  } else {
    for (auto i = 0; i < getReorder().size(); ++i)
      invReorder[getReorder()[i]] = i;
  }
  return invReorder;
}

llvm::TypeSize
StructType::getTypeSizeInBits(const mlir::DataLayout &dl,
                              mlir::DataLayoutEntryListRef params) const {
  unsigned stSize = 0;
  uint64_t stAlign = 1;

  for (int32_t i : getInverseReorder()) {
    mlir::Type ty = getMembers()[i];
    uint64_t tyAlign = dl.getTypeABIAlignment(ty);

    stSize = llvm::alignTo(stSize, tyAlign);
    stSize += dl.getTypeSize(ty).getFixedValue();

    stAlign = std::max(tyAlign, stAlign);
  }

  stSize = llvm::alignTo(stSize, stAlign);
  return llvm::TypeSize::getFixed(stSize * 8); // We want in bits.
}

uint64_t
StructType::getABIAlignment(const mlir::DataLayout &dl,
                            mlir::DataLayoutEntryListRef params) const {
  uint64_t stAlign = 1;

  for (mlir::Type ty : getMembers()) {
    uint64_t tyAlign = dl.getTypeABIAlignment(ty);
    stAlign = std::max(tyAlign, stAlign);
  }

  return stAlign;
}

} // namespace bir
} // namespace belalang
