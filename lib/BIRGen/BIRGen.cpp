#include "belalang/BIRGen/BIRGen.h"

#include "belalang/AST/Decl.h"
#include "belalang/AST/Expr.h"
#include "belalang/AST/Stmt.h"
#include "belalang/BIR/IR/BIR.h"
#include "belalang/BIR/Passes.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"
#include "mlir/IR/DialectRegistry.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Pass/PassManager.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"

namespace belalang {
namespace birgen {

using namespace ast;
using namespace lexer;

BIRGen::BIRGen(diag::DiagnosticEngine &diagEngine)
    : diagEngine(diagEngine), typeChecker(diagEngine), builder(&context),
      loc(builder.getUnknownLoc()) {
  // Load dialects.
  mlir::DialectRegistry registry;
  registry.insert<bir::BIRDialect, mlir::LLVM::LLVMDialect>();
  context.appendDialectRegistry(registry);
  context.getOrLoadDialect<bir::BIRDialect>();

  // Create the module.
  module = mlir::ModuleOp::create(loc);
  builder.setInsertionPointToStart(module.getBody());

  // Create the main function.
  auto retTy = bir::IntType::get(&context);
  auto main = bir::FuncOp::create(
      builder, loc, "main", mlir::FunctionType::get(&context, {}, {retTy}));
  mlir::Block *entry = main.addEntryBlock();
  builder.setInsertionPointToStart(entry);
}

mlir::ModuleOp BIRGen::generateProgram(Program *prog) {
  visitProgram(prog);
  buildMainReturn();
  return module;
}

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------

mlir::Type BIRGen::mapTypeName(llvm::StringRef name) {
  if (name == "String")
    return bir::StringType::get(&context);
  if (name == "Int")
    return bir::IntType::get(&context);
  if (name == "Float")
    return bir::FloatType::get(&context);
  if (name == "Bool")
    return bir::BoolType::get(&context);

  llvm_unreachable("unknown type");
}

static mlir::Type mapType(mlir::MLIRContext *context, Type ty) {
  switch (ty) {
  case Type::Integer:
    return bir::IntType::get(context);
  case Type::Boolean:
    return bir::BoolType::get(context);
  case Type::String:
    return bir::StringType::get(context);
  case Type::Float:
    return bir::FloatType::get(context);
  default:
    return nullptr;
  }
}

namespace {

template <typename Op>
mlir::Value buildBinop(mlir::OpBuilder &builder, mlir::Location loc,
                       mlir::Value lhs, mlir::Value rhs) {
  auto type = lhs.getType();
  auto op = Op::create(builder, loc, type, lhs, rhs);
  return op.getResult();
}

mlir::Value buildBinopCmp(mlir::OpBuilder &builder, mlir::Location loc,
                          bir::CmpOpKind kind, mlir::Value lhs,
                          mlir::Value rhs) {
  auto op = bir::CmpOp::create(builder, loc, kind, lhs, rhs);
  return op.getResult();
}

mlir::Value buildBinopArith(mlir::OpBuilder &builder, mlir::Location loc,
                            TokenKind op, mlir::Value lhs, mlir::Value rhs) {
  switch (op) {
  case TokenKind::Add:
    return buildBinop<bir::AddOp>(builder, loc, lhs, rhs);
  case TokenKind::Sub:
    return buildBinop<bir::SubOp>(builder, loc, lhs, rhs);
  case TokenKind::Mul:
    return buildBinop<bir::MulOp>(builder, loc, lhs, rhs);
  case TokenKind::Div:
    return buildBinop<bir::DivOp>(builder, loc, lhs, rhs);
  case TokenKind::Mod:
    return buildBinop<bir::ModOp>(builder, loc, lhs, rhs);
  case TokenKind::Lt:
    return buildBinopCmp(builder, loc, bir::CmpOpKind::lt, lhs, rhs);
  case TokenKind::Le:
    return buildBinopCmp(builder, loc, bir::CmpOpKind::le, lhs, rhs);
  case TokenKind::Gt:
    return buildBinopCmp(builder, loc, bir::CmpOpKind::gt, lhs, rhs);
  case TokenKind::Ge:
    return buildBinopCmp(builder, loc, bir::CmpOpKind::ge, lhs, rhs);
  case TokenKind::Eq:
    return buildBinopCmp(builder, loc, bir::CmpOpKind::eq, lhs, rhs);
  case TokenKind::Ne:
    return buildBinopCmp(builder, loc, bir::CmpOpKind::ne, lhs, rhs);
  default:
    llvm_unreachable("unsupported infix operator");
  }
}

} // namespace

// -----------------------------------------------------------------------------
// Statements
// -----------------------------------------------------------------------------

mlir::Value BIRGen::visitReturnStmt(ReturnStmt *stmt) {
  if (Expr *returnValue = stmt->getReturnValue()) {
    mlir::Value val = visitExpr(returnValue);
    bir::ReturnOp::create(builder, loc, val);
  } else {
    bir::ReturnOp::create(builder, loc, {});
  }
  return {};
}

mlir::Value BIRGen::visitWhileStmt(WhileStmt *stmt) {
  auto whileOp = bir::WhileOp::create(builder, loc);

  builder.createBlock(&whileOp.getCond());
  mlir::Value cond = visitExpr(stmt->getCond());
  bir::ConditionOp::create(builder, loc, cond);

  builder.createBlock(&whileOp.getBody());

  auto scopeOp =
      bir::ScopeOp::create(builder, loc, llvm::SmallVector<mlir::Type, 0>{});
  builder.createBlock(&scopeOp.getScopeRegion());

  for (Stmt *s : stmt->getBody()->getStmts()) {
    visitStmt(s);
  }

  bir::YieldOp::create(builder, loc);

  builder.setInsertionPointAfter(scopeOp);
  bir::YieldOp::create(builder, loc);

  builder.setInsertionPointAfter(whileOp);
  return {};
}

mlir::Value BIRGen::visitBreakStmt(BreakStmt *stmt) {
  bir::BreakOp::create(builder, loc);
  return {};
}

mlir::Value BIRGen::visitContinueStmt(ContinueStmt *stmt) {
  bir::ContinueOp::create(builder, loc);
  return {};
}

mlir::Value BIRGen::visitImportStmt(ImportStmt *stmt) {
  llvm_unreachable("not yet implemented");
}

mlir::Value BIRGen::visitExprStmt(ExprStmt *stmt) {
  visitExpr(stmt->getExpr());
  return {};
}

mlir::Value BIRGen::visitDeclStmt(DeclStmt *stmt) {
  visitDecl(stmt->getDecl());
  return {};
}

// -----------------------------------------------------------------------------
// Expressions
// -----------------------------------------------------------------------------

mlir::Value BIRGen::visitIntLitExpr(IntLitExpr *expr) {
  auto type = builder.getType<bir::IntType>();
  llvm::APInt value(64, expr->getValue());
  auto attr = bir::IntegerAttr::get(&context, type, value);
  return bir::ConstantOp::create(builder, loc, type, attr).getResult();
}

mlir::Value BIRGen::visitFloatLitExpr(FloatLitExpr *expr) {
  auto type = builder.getType<bir::FloatType>();
  llvm::APFloat value(expr->getValue());
  auto attr = bir::FloatAttr::get(&context, type, value);
  return bir::ConstantOp::create(builder, loc, type, attr).getResult();
}

mlir::Value BIRGen::visitStringLitExpr(StringLitExpr *expr) {
  auto type = builder.getType<bir::StringType>();
  auto attr = bir::StringAttr::get(&context, type, expr->getValue());
  return bir::ConstantOp::create(builder, loc, type, attr).getResult();
}

mlir::Value BIRGen::visitBoolExpr(BoolExpr *expr) {
  auto type = builder.getType<bir::BoolType>();
  auto attr = bir::BoolAttr::get(&context, type, expr->getValue());
  return bir::ConstantOp::create(builder, loc, type, attr).getResult();
}

mlir::Value BIRGen::visitArrayLitExpr(ArrayLitExpr *expr) {
  llvm_unreachable("not yet implemented");
}

mlir::Value BIRGen::visitVarExpr(VarExpr *expr) {
  auto it = symbolTable.find(expr->getName());
  if (it == symbolTable.end()) {
    llvm_unreachable("variable not found");
  }

  mlir::Value dest = it->second;
  mlir::Value rhs = visitExpr(expr->getValue());

  if (expr->getAssignOp() == TokenKind::Assign) {
    bir::StoreOp::create(builder, loc, rhs, dest);
    return rhs;
  }

  auto refType = llvm::cast<bir::RefType>(dest.getType());
  auto loadOp =
      bir::VarLoadOp::create(builder, loc, refType.getReferent(), dest);
  mlir::Value lhs = loadOp.getResult();

  mlir::Value res;
  switch (expr->getAssignOp()) {
  case TokenKind::AddAssign:
    res = bir::AddOp::create(builder, loc, lhs.getType(), lhs, rhs);
    break;
  case TokenKind::SubAssign:
    res = bir::SubOp::create(builder, loc, lhs.getType(), lhs, rhs);
    break;
  case TokenKind::MulAssign:
    res = bir::MulOp::create(builder, loc, lhs.getType(), lhs, rhs);
    break;
  case TokenKind::DivAssign:
    res = bir::DivOp::create(builder, loc, lhs.getType(), lhs, rhs);
    break;
  default:
    llvm_unreachable("unknown assign op");
  }

  bir::StoreOp::create(builder, loc, res, dest);
  return res;
}

mlir::Value BIRGen::visitFunctionLitExpr(FunctionLitExpr *expr) {
  if (expr->getExplicitType().empty())
    llvm_unreachable("not yet implemented");

  mlir::Type ty = mapTypeName(expr->getExplicitType());
  llvm::SmallVector<mlir::Type, 0> paramsTy;

  for (VarDecl *param : expr->getParams()) {
    paramsTy.push_back(mapTypeName(param->getExplicitType()));
  }

  auto fnTy = mlir::FunctionType::get(&context, paramsTy, {ty});
  auto op = bir::FuncExprOp::create(builder, loc, fnTy);

  {
    mlir::OpBuilder::InsertionGuard g(builder);
    llvm::SmallVector<mlir::Location> locs(paramsTy.size(), loc);
    builder.createBlock(&op.getBody(), {}, paramsTy, locs);

    auto args = op.getBody().front().getArguments();
    for (size_t i = 0; i < args.size(); ++i) {
      VarDecl *paramDecl = expr->getParams()[i];
      mlir::Type paramType = args[i].getType();
      auto refType = bir::RefType::get(&context, paramType);
      auto dest =
          bir::DeclareOp::create(builder, loc, refType, paramDecl->getName());
      bir::StoreOp::create(builder, loc, args[i], dest);
      symbolTable[paramDecl->getName()] = dest;
    }

    for (Stmt *stmt : expr->getBody()->getStmts()) {
      visitStmt(stmt);
    }
  }

  return op.getResult();
}

mlir::Value BIRGen::visitCallExpr(CallExpr *expr) {
  if (auto *ident = llvm::dyn_cast<IdentifierExpr>(expr->getCallee())) {
    if (ident->getName() == "print") {
      mlir::Value value = visitExpr(expr->getArgs()[0]);
      bir::PrintOp::create(builder, loc, value);
      return {};
    }
  }

  mlir::Value callee = visitExpr(expr->getCallee());
  llvm::SmallVector<mlir::Value, 4> operands;
  for (Expr *arg : expr->getArgs()) {
    operands.push_back(visitExpr(arg));
  }

  auto fnType = llvm::cast<mlir::FunctionType>(callee.getType());
  auto resultTypes = fnType.getResults();

  auto callOp = bir::CallIndirectOp::create(builder, loc, resultTypes, callee,
                                            operands, nullptr, nullptr);

  if (resultTypes.empty())
    return {};
  return callOp.getResult(0);
}

mlir::Value BIRGen::visitIndexExpr(IndexExpr *expr) {
  llvm_unreachable("not yet implemented");
}

mlir::Value BIRGen::visitIdentifierExpr(IdentifierExpr *expr) {
  auto it = symbolTable.find(expr->getName());
  if (it == symbolTable.end()) {
    llvm_unreachable("unknown identifier");
  }

  mlir::Value ref = it->second;
  auto refType = llvm::cast<bir::RefType>(ref.getType());
  auto loadOp =
      bir::VarLoadOp::create(builder, loc, refType.getReferent(), ref);
  return loadOp.getResult();
}

mlir::Value BIRGen::visitIfExpr(IfExpr *expr) {
  mlir::Value cond = visitExpr(expr->getCond());

  Type inferredTy = typeChecker.visitIfExpr(expr);
  llvm::SmallVector<mlir::Type, 1> resultTypes;
  if (inferredTy != Type::None) {
    if (auto ty = mapType(&context, inferredTy)) {
      resultTypes.push_back(ty);
    }
  }

  auto ifOp = bir::IfOp::create(builder, loc, resultTypes, cond);

  builder.createBlock(&ifOp.getThenRegion());
  mlir::Value thenVal = nullptr;
  for (Stmt *stmt : expr->getThen()->getStmts()) {
    if (auto *exprStmt = llvm::dyn_cast<ExprStmt>(stmt)) {
      thenVal = visitExpr(exprStmt->getExpr());
    } else {
      visitStmt(stmt);
      thenVal = nullptr;
    }
  }
  if (thenVal && !resultTypes.empty()) {
    bir::YieldOp::create(builder, loc, thenVal);
  } else {
    bir::YieldOp::create(builder, loc);
  }

  if (Expr *altExpr = expr->getAlt()) {
    builder.createBlock(&ifOp.getElseRegion());
    mlir::Value elseVal = nullptr;
    if (auto *altBlock = llvm::dyn_cast<BlockExpr>(altExpr)) {
      for (Stmt *stmt : altBlock->getStmts()) {
        if (auto *exprStmt = llvm::dyn_cast<ExprStmt>(stmt)) {
          elseVal = visitExpr(exprStmt->getExpr());
        } else {
          visitStmt(stmt);
          elseVal = nullptr;
        }
      }
    } else {
      elseVal = visitExpr(altExpr);
    }
    if (elseVal && !resultTypes.empty()) {
      bir::YieldOp::create(builder, loc, elseVal);
    } else {
      bir::YieldOp::create(builder, loc);
    }
  } else {
    builder.createBlock(&ifOp.getElseRegion());
    bir::YieldOp::create(builder, loc);
  }

  builder.setInsertionPointAfter(ifOp);
  if (resultTypes.empty())
    return {};
  return ifOp.getResult();
}

mlir::Value BIRGen::visitInfixExpr(InfixExpr *expr) {
  mlir::Value lhs = visitExpr(expr->getLeft());
  mlir::Value rhs = visitExpr(expr->getRight());
  return buildBinopArith(builder, loc, expr->getInfixOp(), lhs, rhs);
}

mlir::Value BIRGen::visitPrefixExpr(PrefixExpr *expr) {
  llvm_unreachable("not yet implemented");
}

mlir::Value BIRGen::visitBlockExpr(BlockExpr *expr) {
  Type inferredTy = typeChecker.visitBlockExpr(expr);
  llvm::SmallVector<mlir::Type, 1> resultTypes;
  if (inferredTy != Type::None) {
    if (auto ty = mapType(&context, inferredTy)) {
      resultTypes.push_back(ty);
    }
  }

  auto scopeOp = bir::ScopeOp::create(builder, loc, resultTypes);
  builder.createBlock(&scopeOp.getScopeRegion());

  mlir::Value lastVal;
  for (Stmt *stmt : expr->getStmts()) {
    if (auto *exprStmt = llvm::dyn_cast<ExprStmt>(stmt)) {
      lastVal = visitExpr(exprStmt->getExpr());
    } else {
      visitStmt(stmt);
      lastVal = nullptr;
    }
  }

  if (lastVal && !resultTypes.empty()) {
    bir::YieldOp::create(builder, loc, lastVal);
  } else {
    bir::YieldOp::create(builder, loc);
  }

  builder.setInsertionPointAfter(scopeOp);
  if (resultTypes.empty())
    return {};
  return scopeOp.getResult(0);
}

mlir::Value BIRGen::visitMemberAccessExpr(MemberAccessExpr *expr) {
  llvm_unreachable("not yet implemented");
}

mlir::Value BIRGen::visitStructLiteralExpr(StructLiteralExpr *expr) {
  llvm_unreachable("not yet implemented");
}

// -----------------------------------------------------------------------------
// Declarations
// -----------------------------------------------------------------------------

mlir::Value BIRGen::visitVarDecl(VarDecl *decl) {
  Expr *value = decl->getValue();
  typeChecker.visitVarDecl(decl);

  mlir::Type type;
  mlir::Value src;
  if (value) {
    src = visitExpr(value);
    type = src.getType();
  } else {
    type = mapTypeName(decl->getExplicitType());
  }

  auto refType = bir::RefType::get(&context, type);
  auto dest = bir::DeclareOp::create(builder, loc, refType, decl->getName());
  if (src) {
    bir::StoreOp::create(builder, loc, src, dest);
  }
  symbolTable[decl->getName()] = dest;
  return dest;
}

mlir::Value BIRGen::visitStructDecl(StructDecl *decl) {
  llvm_unreachable("not yet implemented");
}

// -----------------------------------------------------------------------------
// Module
// -----------------------------------------------------------------------------

void BIRGen::buildMainReturn() {
  auto typ = bir::IntType::get(&context);
  auto val = llvm::APInt(64, 0); // return 0
  auto atr = bir::IntegerAttr::get(&context, typ, val);
  auto ret = bir::ConstantOp::create(builder, loc, typ, atr);
  bir::ReturnOp::create(builder, loc, {ret});
}

bool BIRGen::runLoweringPipeline() {
  mlir::PassManager pm(&context);
  bir::buildBIRLoweringPipeline(pm);
  return mlir::succeeded(pm.run(module));
}

std::string BIRGen::dumpToString() const {
  std::string s;
  llvm::raw_string_ostream os(s);
  const_cast<mlir::ModuleOp &>(module).print(os);
  return os.str();
}

} // namespace birgen
} // namespace belalang
