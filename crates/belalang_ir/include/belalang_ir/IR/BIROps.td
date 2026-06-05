#ifndef BELALANG_IR_OPS_TD_
#define BELALANG_IR_OPS_TD_

include "belalang_ir/IR/BIRDialect.td"
include "belalang_ir/IR/BIRTypes.td"

include "mlir/IR/BuiltinAttributeInterfaces.td"

class BIR_Op<string mnemonic, list<Trait> traits = []> :
    Op<BIR_Dialect, mnemonic, traits>;

def MinimalOp : BIR_Op<"minimal"> {
  let summary = "minimal";

  let arguments = (ins);
  let results = (outs);

  let assemblyFormat = "attr-dict";
}

def ConstantOp : BIR_Op<"constant", [AllTypesMatch<["value", "result"]>]> {
  let summary = "constant";

  let arguments = (ins TypedAttrInterface:$value);
  let results = (outs AnyType:$result);

  let assemblyFormat = "attr-dict $value";
}

#endif // BELALANG_IR_OPS_TD_
