#ifndef BELALANG_GCIR_IR_GCIRDIALECT_TD_
#define BELALANG_GCIR_IR_GCIRDIALECT_TD_

include "mlir/IR/OpBase.td"

def GCIR_Dialect : Dialect {
  let name = "gc";
  let cppNamespace = "::belalang::gc";

  let useDefaultTypePrinterParser = 1;
}

#endif // BELALANG_GCIR_IR_GCIRDIALECT_TD_
