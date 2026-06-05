#include "belalang_ir/IR/BIRDialect.h"

#include "belalang_ir/IR/BIRDialect.cpp.inc"

#define GET_OP_CLASSES
#include "belalang_ir/IR/BIROps.cpp.inc"

namespace bir {

void BIRDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "belalang_ir/IR/BIROps.cpp.inc"
      >();
}

} // namespace bir
