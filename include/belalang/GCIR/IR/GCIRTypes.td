#ifndef BELALANG_GCIR_IR_GCIRTYPES_TD_
#define BELALANG_GCIR_IR_GCIRTYPES_TD_

include "belalang/GCIR/IR/GCIRDialect.td"

class GCIR_Type<string name, string typeMnemonic,
                 list<Trait> traits = []>
    : TypeDef<GCIR_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def GCIR_PtrType : GCIR_Type<"Ptr", "ptr"> {
  let summary = "Movable garbage-collected pointer";

  let parameters = (ins
    "::mlir::Type":$pointee
  );

  let assemblyFormat = "`<` $pointee `>`";
}

#endif // BELALANG_GCIR_IR_GCIRTYPES_TD_
