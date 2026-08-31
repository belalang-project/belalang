#include "belalang/GCIR/IR/GCIR.h"

#include "mlir/IR/DialectImplementation.h"

#include "belalang/GCIR/IR/GCIRDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "belalang/GCIR/IR/GCIRTypes.cpp.inc"

namespace belalang::gc {

void GCIRDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "belalang/GCIR/IR/GCIRTypes.cpp.inc"
      >();
}

} // namespace belalang::gc
