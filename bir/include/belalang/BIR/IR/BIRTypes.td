#ifndef BELALANG_IR_TYPES_TD_
#define BELALANG_IR_TYPES_TD_

include "belalang/BIR/IR/BIRDialect.td"

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

def BIR_StringType : BIR_Type<"String", "string"> {
  let summary = "String type";
}

def BIR_RefType : BIR_Type<"Ref", "ref"> {
  let summary = "Reference type";

  let parameters = (ins AnyTypeOf<[BIR_IntType, BIR_FloatType]>:$el);

  let assemblyFormat = "`<` $el `>`";
}

#endif // BELALANG_IR_TYPES_TD_
