#include "belalang/IR/BIRDialect.h"

#include "belalang/IR/BIRDialect.cpp.inc"

namespace belalang {
namespace bir {

void BIRDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "belalang/IR/BIRTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "belalang/IR/BIROps.cpp.inc"
      >();
}

} // namespace bir
} // namespace belalang
