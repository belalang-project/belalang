#ifndef BELALANG_GCIR_IR_GCIR_H_
#define BELALANG_GCIR_IR_GCIR_H_

#include "mlir/IR/Builders.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpImplementation.h"
#include "mlir/IR/Types.h"

#include "llvm/ADT/TypeSwitch.h"

#include "belalang/GCIR/IR/GCIRDialect.h.inc"

#define GET_TYPEDEF_CLASSES
#include "belalang/GCIR/IR/GCIRTypes.h.inc"

#endif // BELALANG_GCIR_IR_GCIR_H_
