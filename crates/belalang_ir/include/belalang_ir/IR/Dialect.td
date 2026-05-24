#ifndef BELALANG_IR_DIALECT_TD_
#define BELALANG_IR_DIALECT_TD_

include "mlir/IR/OpBase.td"

def BIR_Dialect : Dialect {
  let name = "bir";
  let cppNamespace = "::bir";
}

#endif // BELALANG_IR_DIALECT_TD_
