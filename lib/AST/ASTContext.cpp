#include "belalang/AST/ASTContext.h"

#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Casting.h"

namespace belalang {
namespace ast {

ASTContext::ASTContext() { initBuiltinTypes(); }

void ASTContext::initBuiltinType(BuiltinType *&r, BuiltinType::Kind kind) {
  auto *ty = new (*this) BuiltinType(kind);
  r = ty;
  types.push_back(ty);
}

void ASTContext::initBuiltinTypes() {
  initBuiltinType(intTy, BuiltinType::Kind::Integer);
  initBuiltinType(floatTy, BuiltinType::Kind::Float);
  initBuiltinType(boolTy, BuiltinType::Kind::Boolean);
  initBuiltinType(stringTy, BuiltinType::Kind::String);
  initBuiltinType(noneTy, BuiltinType::Kind::None);
}

llvm::StringRef ASTContext::tyToStr(Type *ty) const {
  if (auto *builtinTy = llvm::dyn_cast<BuiltinType>(ty)) {
    if (builtinTy == stringTy)
      return "String";
    if (builtinTy == intTy)
      return "Int";
    if (builtinTy == floatTy)
      return "Float";
    if (builtinTy == boolTy)
      return "Bool";
    if (builtinTy == noneTy)
      return "None";
  }
  return "None";
}

Type *ASTContext::strToTy(llvm::StringRef str) const {
  return llvm::StringSwitch<Type *>(str)
      .Case("Int", intTy)
      .Case("Float", floatTy)
      .Case("String", stringTy)
      .Case("Bool", boolTy)
      .Case("None", noneTy)
      .Default(noneTy);
}

} // namespace ast
} // namespace belalang
