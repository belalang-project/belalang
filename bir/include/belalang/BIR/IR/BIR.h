#ifndef BELALANG_IR_DIALECT_H_
#define BELALANG_IR_DIALECT_H_

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/SymbolTable.h"
#include "mlir/Interfaces/FunctionInterfaces.h"
#include "mlir/Support/LLVM.h"

#include "llvm/ADT/TypeSwitch.h"

#include "belalang/BIR/Interfaces/LoopOpInterface.h"

using namespace mlir;
using namespace llvm;

#include "belalang/BIR/IR/BIRDialect.h.inc"

#define GET_ATTRDEF_CLASSES
#include "belalang/BIR/IR/BIRAttrs.h.inc"

#define GET_TYPEDEF_CLASSES
#include "belalang/BIR/IR/BIRTypes.h.inc"

#define GET_OP_CLASSES
#include "belalang/BIR/IR/BIROps.h.inc"

#endif // BELALANG_IR_DIALECT_H_
