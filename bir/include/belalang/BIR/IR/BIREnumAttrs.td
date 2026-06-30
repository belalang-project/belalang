#ifndef BELALANG_BIR_IR_BIRENUM_ATTRS_TD_
#define BELALANG_BIR_IR_BIRENUM_ATTRS_TD_

include "mlir/IR/EnumAttr.td"

class BIR_I32EnumAttr<string name, string summary, list<I32EnumAttrCase> cases>
    : I32EnumAttr<name, summary, cases> {
  let cppNamespace = "::belalang::bir";
}

#endif // BELALANG_BIR_IR_BIRENUM_ATTRS_TD_
