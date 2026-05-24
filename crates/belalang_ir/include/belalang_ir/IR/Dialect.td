#ifndef BELALANG_IR_DIALECT
#define BELALANG_IR_DIALECT

include "mlir/IR/OpBase.td"

def BelalangDialect : Dialect {
  let name = "belalang";
  let cppNamespace = "::mlir::belalang";
}

#endif // BELALANG_IR_DIALECT
