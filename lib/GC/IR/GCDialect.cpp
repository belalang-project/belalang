#include "mlir/Dialect/GC/IR/GC.h"

#include "mlir/IR/DialectImplementation.h"

#include "mlir/Dialect/GC/IR/GCDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "mlir/Dialect/GC/IR/GCTypes.cpp.inc"

#define GET_OP_CLASSES
#include "mlir/Dialect/GC/IR/GCOps.cpp.inc"

namespace mlir::gc {

void GCDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "mlir/Dialect/GC/IR/GCTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "mlir/Dialect/GC/IR/GCOps.cpp.inc"
      >();
}

} // namespace mlir::gc
