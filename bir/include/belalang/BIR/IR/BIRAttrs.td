#ifndef BELALANG_BIR_IR_BIRATTRS_TD_
#define BELALANG_BIR_IR_BIRATTRS_TD_

include "belalang/BIR/IR/BIRDialect.td"

include "mlir/IR/AttrTypeBase.td"
include "mlir/IR/BuiltinAttributeInterfaces.td"

class BIR_Attr<string name, string attrMnemonic, list<Trait> traits = []>
    : AttrDef<BIR_Dialect, name, traits> {
  let mnemonic = attrMnemonic;
}

def BIR_IntegerAttr : BIR_Attr<"Integer", "int", [TypedAttrInterface]> {
  let summary = "An attribute containing an integer value";

  let parameters = (ins
    AttributeSelfTypeParameter<"">:$type,
    APIntParameter<"">:$value
  );

  let hasCustomAssemblyFormat = 1;
}

def BIR_FloatAttr : BIR_Attr<"Float", "float", [TypedAttrInterface]> {
  let summary = "An attribute containing an floating-point value";

  let parameters = (ins
    AttributeSelfTypeParameter<"">:$type,
    APFloatParameter<"">:$value
  );

  let hasCustomAssemblyFormat = 1;
}

#endif // BELALANG_BIR_IR_BIRATTRS_TD_
