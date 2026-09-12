#ifndef MLIR_DIALECT_GC_IR_GCDIALECT_TD_
#define MLIR_DIALECT_GC_IR_GCDIALECT_TD_

include "mlir/IR/OpBase.td"

def GC_Dialect : Dialect {
  let name = "gc";
  let cppNamespace = "::mlir::gc";

  let useDefaultTypePrinterParser = 1;
}

#endif // MLIR_DIALECT_GC_IR_GCDIALECT_TD_
