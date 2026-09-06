#ifndef BELALANG_GCIR_IR_GCIROPS_TD_
#define BELALANG_GCIR_IR_GCIROPS_TD_

include "belalang/GCIR/IR/GCIRDialect.td"
include "belalang/GCIR/IR/GCIRTypes.td"

class GCIR_Op<string mnemonic, list<Trait> traits = []>
    : Op<GCIR_Dialect, mnemonic, traits> {}

def GCIR_AllocOp : GCIR_Op<"alloc"> {
  let results = (outs
    GCIR_PtrType:$result
  );

  let assemblyFormat = [{
    `:` qualified(type($result))
    attr-dict
  }];
}

#endif // BELALANG_GCIR_IR_GCIROPS_TD_
