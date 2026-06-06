#ifndef BELALANG_IR_DIALECT_H_
#define BELALANG_IR_DIALECT_H_

#include "mlir/Bytecode/BytecodeOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"

#include "belalang/IR/BIRDialect.h.inc"

#define GET_OP_CLASSES
#include "belalang/IR/BIROps.h.inc"

#define GET_TYPEDEF_CLASSES
#include "belalang/IR/BIRTypes.h.inc"

#endif // BELALANG_IR_DIALECT_H_
