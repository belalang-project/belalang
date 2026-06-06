#ifndef BELALANG_IR_TYPES_TD_
#define BELALANG_IR_TYPES_TD_

include "belalang_ir/IR/BIRDialect.td"

class BIR_Type<string name, string typeMnemonic, list<Trait> traits = [],
               string baseCppClass = "::mlir::Type">
    : TypeDef<BIR_Dialect, name, traits, baseCppClass> {
  let mnemonic = typeMnemonic;
}

def BIR_IntType : BIR_Type<"Int", "int"> {
  let summary = "Integer number type";
}

def BIR_FloatType : BIR_Type<"Float", "float"> {
  let summary = "Floating-point number type";
}

#endif // BELALANG_IR_TYPES_TD_
