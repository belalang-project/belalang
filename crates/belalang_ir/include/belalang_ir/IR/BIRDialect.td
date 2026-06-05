#ifndef BELALANG_IR_DIALECT_TD_
#define BELALANG_IR_DIALECT_TD_

include "mlir/IR/OpBase.td"

def BIR_Dialect : Dialect {
  let name = "bir";
  let cppNamespace = "::bir";

  let useDefaultTypePrinterParser = 1;
}

#endif // BELALANG_IR_DIALECT_TD_
