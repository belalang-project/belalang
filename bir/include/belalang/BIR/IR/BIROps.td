#ifndef BELALANG_IR_OPS_TD_
#define BELALANG_IR_OPS_TD_

include "belalang/BIR/IR/BIRDialect.td"
include "belalang/BIR/IR/BIRTypes.td"

include "mlir/IR/BuiltinAttributeInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"

class BIR_Op<string mnemonic, list<Trait> traits = []>
    : Op<BIR_Dialect, mnemonic, traits>;

def BIR_AnyType : AnyTypeOf<[BIR_IntType, BIR_FloatType]>;

def BIR_ConstantOp : BIR_Op<"constant"> {
  let summary = "constant";

  let description = [{
    ```mlir
    %0 = bir.constant 1 : !bir.int
    ```
  }];

  let arguments = (ins TypedAttrInterface:$value);
  let results = (outs BIR_AnyType:$result);

  let hasCustomAssemblyFormat = 1;
}

def BIR_AddOp : BIR_Op<"add"> {
  let summary = "add";

  let description = [{
    ```mlir
    %0 = bir.constant 4 : !bir.int
    %1 = bir.constant 2 : !bir.int
    %2 = bir.add %0, %1 : (!bir.int, !bir.int) -> !bir.int
    ```
  }];

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_SubOp : BIR_Op<"sub"> {
  let summary = "sub";

  let description = [{
    ```mlir
    %0 = bir.constant 4 : !bir.int
    %1 = bir.constant 2 : !bir.int
    %2 = bir.sub %0, %1 : (!bir.int, !bir.int) -> !bir.int
    ```
  }];

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_MulOp : BIR_Op<"mul"> {
  let summary = "mul";

  let description = [{
    ```mlir
    %0 = bir.constant 4 : !bir.int
    %1 = bir.constant 2 : !bir.int
    %2 = bir.mul %0, %1 : (!bir.int, !bir.int) -> !bir.int
    ```
  }];

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_DivOp : BIR_Op<"div"> {
  let summary = "div";

  let description = [{
    ```mlir
    %0 = bir.constant 4 : !bir.int
    %1 = bir.constant 2 : !bir.int
    %2 = bir.div %0, %1 : (!bir.int, !bir.int) -> !bir.int
    ```
  }];

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_ModOp : BIR_Op<"mod"> {
  let summary = "mod";

  let description = [{
    ```mlir
    %0 = bir.constant 4 : !bir.int
    %1 = bir.constant 2 : !bir.int
    %2 = bir.mod %0, %1 : (!bir.int, !bir.int) -> !bir.int
    ```
  }];

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_PrintOp : BIR_Op<"print"> {
  let summary = "print";

  let description = [{
    ```mlir
    %0 = bir.constant 0 : !bir.int
    bir.print %0 : !bir.int
    ```
  }];

  let arguments = (ins BIR_AnyType:$value);

  let assemblyFormat = "attr-dict $value `:` type($value)";
}

def FuncOp : BIR_Op<"func"> {
  let summary = "func";

  let regions = (region AnyRegion:$body);

  let assemblyFormat = "attr-dict-with-keyword $body";
}

def ReturnOp : BIR_Op<"return", [ParentOneOf<["FuncOp"]>, Terminator]> {
  let summary = "return";

  let assemblyFormat = "attr-dict";
}

#endif // BELALANG_IR_OPS_TD_
