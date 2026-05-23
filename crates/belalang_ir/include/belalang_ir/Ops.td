#ifndef BELALANG_IR_OPS
#define BELALANG_IR_OPS

include "mlir/IR/OpBase.td"

def BelalangDialect : Dialect {
  let name = "belalang";
  let cppNamespace = "::mlir::belalang";
}

#endif // BELALANG_IR_OPS
