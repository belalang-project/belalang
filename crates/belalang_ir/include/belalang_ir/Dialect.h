#ifndef BELALANG_IR_DIALECT_H_
#define BELALANG_IR_DIALECT_H_

#include "mlir/IR/Dialect.h"

#include "belalang_ir/Dialect.h.inc"

#define GET_OP_CLASSES
#include "belalang_ir/Ops.h.inc"

#endif // BELALANG_IR_DIALECT_H_
