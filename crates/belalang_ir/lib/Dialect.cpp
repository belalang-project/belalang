#include "belalang_ir/IR/Dialect.h"

#include "belalang_ir/IR/Dialect.cpp.inc"

#define GET_OP_CLASSES
#include "belalang_ir/IR/Ops.cpp.inc"

namespace bir {

void BIRDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "belalang_ir/IR/Ops.cpp.inc"
      >();
}

} // namespace bir
