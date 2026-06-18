#ifndef BELALANG_BIR_IR_BIRATTRS_TD_
#define BELALANG_BIR_IR_BIRATTRS_TD_

include "mlir/IR/AttrTypeBase.td"
include "belalang/BIR/IR/BIRDialect.td"

class BIR_Attr<string name, string attrMnemonic, list<Trait> traits = []>
    : AttrDef<BIR_Dialect, name, traits> {
  let mnemonic = attrMnemonic;
}

def BIR_IntegerAttr : BIR_Attr<"Integer", "int"> {
  let summary = "An attribute containing an integer value";

  let parameters = (ins
    AttributeSelfTypeParameter<"">:$type,
    APIntParameter<"">:$value
  );

  let assemblyFormat = "`<` custom<IntLiteral>($value, $type) `>`";
}

def BIR_FloatAttr : BIR_Attr<"Float", "float"> {
  let summary = "An attribute containing an floating-point value";

  let parameters = (ins
    AttributeSelfTypeParameter<"">:$type,
    APFloatParameter<"">:$value
  );

  let assemblyFormat = "`<` custom<FloatLiteral>($value, $type) `>`";
}

#endif // BELALANG_BIR_IR_BIRATTRS_TD_
