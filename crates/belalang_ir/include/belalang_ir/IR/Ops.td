#ifndef BELALANG_IR_OPS_TD_
#define BELALANG_IR_OPS_TD_

include "belalang_ir/IR/Dialect.td"

class BIR_Op<string mnemonic, list<Trait> traits = []> :
    Op<BIR_Dialect, mnemonic, traits>;

def MinimalOp : BIR_Op<"minimal"> {
  let summary = "minimal";

  let arguments = (ins);
  let results = (outs);

  let assemblyFormat = "attr-dict";
}

#endif // BELALANG_IR_OPS_TD_
