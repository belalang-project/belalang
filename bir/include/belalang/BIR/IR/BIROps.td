#ifndef BELALANG_IR_OPS_TD_
#define BELALANG_IR_OPS_TD_

include "belalang/BIR/IR/BIRDialect.td"
include "belalang/BIR/IR/BIRTypes.td"

include "mlir/IR/BuiltinAttributeInterfaces.td"

class BIR_Op<string mnemonic, list<Trait> traits = []>
    : Op<BIR_Dialect, mnemonic, traits>;

def BIR_AnyType : AnyTypeOf<[BIR_IntType, BIR_FloatType]>;

def MinimalOp : BIR_Op<"minimal"> {
  let summary = "minimal";

  let arguments = (ins);
  let results = (outs);

  let assemblyFormat = "attr-dict";
}

def ConstantOp : BIR_Op<"constant"> {
  let summary = "constant";

  let arguments = (ins TypedAttrInterface:$value);
  let results = (outs BIR_AnyType:$result);

  let hasCustomAssemblyFormat = 1;
}

def AddOp : BIR_Op<"add"> {
  let summary = "add";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def SubOp : BIR_Op<"sub"> {
  let summary = "sub";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def MulOp : BIR_Op<"mul"> {
  let summary = "mul";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def DivOp : BIR_Op<"div"> {
  let summary = "div";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def ModOp : BIR_Op<"mod"> {
  let summary = "mod";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def PrintOp : BIR_Op<"print"> {
  let summary = "print";

  let arguments = (ins BIR_AnyType:$value);

  let assemblyFormat = "attr-dict $value `:` type($value)";
}

#endif // BELALANG_IR_OPS_TD_
