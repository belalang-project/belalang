#include "belalang/BIR/BRTUtils.h"

#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/BuiltinOps.h"

#include <cassert>

void insertBRTInitCall(mlir::ModuleOp module) {
  auto existingInit = module.lookupSymbol<mlir::LLVM::LLVMFuncOp>(kInit);
  auto existingCtor = module.lookupSymbol<mlir::LLVM::LLVMFuncOp>("ctor");
  if (existingInit && existingCtor)
    return;
  assert(!existingCtor && "unexpected ctor function");

  mlir::MLIRContext *ctx = module.getContext();
  mlir::OpBuilder builder(ctx);
  mlir::Location loc = builder.getUnknownLoc();
  mlir::OpBuilder::InsertionGuard guard(builder);
  builder.setInsertionPointToStart(module.getBody());

  llvm::SmallVector<mlir::Attribute> constructors;
  llvm::SmallVector<int32_t> priorities;
  llvm::SmallVector<mlir::Attribute> data;
  auto voidType = mlir::LLVM::LLVMVoidType::get(ctx);
  auto functionType = mlir::LLVM::LLVMFunctionType::get(voidType, {});

  auto init = existingInit;
  if (!init)
    init = mlir::LLVM::LLVMFuncOp::create(builder, loc, kInit, functionType);
  auto ctor = mlir::LLVM::LLVMFuncOp::create(builder, loc, "ctor",
                                             functionType);

  {
    mlir::OpBuilder::InsertionGuard functionGuard(builder);
    mlir::Block *entry = ctor.addEntryBlock(builder);
    builder.setInsertionPointToStart(entry);
    mlir::LLVM::CallOp::create(builder, loc, init, mlir::ValueRange{});
    mlir::LLVM::ReturnOp::create(builder, loc, mlir::ValueRange{});
  }

  constructors.push_back(mlir::FlatSymbolRefAttr::get(ctx, "ctor"));
  priorities.push_back(0);
  data.push_back(mlir::LLVM::ZeroAttr::get(ctx));
  mlir::LLVM::GlobalCtorsOp::create(
      builder, loc, builder.getArrayAttr(constructors),
      builder.getI32ArrayAttr(priorities), builder.getArrayAttr(data));
}
