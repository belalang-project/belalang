#include "belalang_ir/IR/BIRDialect.h"

#include "belalang_ir/IR/BIRDialect.cpp.inc"

namespace bir {

void BIRDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "belalang_ir/IR/BIRTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "belalang_ir/IR/BIROps.cpp.inc"
      >();
}

} // namespace bir
