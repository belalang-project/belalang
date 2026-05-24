#ifndef BELALANG_IR_DIALECT
#define BELALANG_IR_DIALECT

include "mlir/IR/OpBase.td"

def BIR_Dialect : Dialect {
  let name = "bir";
  let cppNamespace = "::bir";
}

#endif // BELALANG_IR_DIALECT
