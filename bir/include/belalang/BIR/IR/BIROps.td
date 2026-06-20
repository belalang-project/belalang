#ifndef BELALANG_IR_OPS_TD_
#define BELALANG_IR_OPS_TD_

include "belalang/BIR/IR/BIRDialect.td"
include "belalang/BIR/IR/BIRTypes.td"

include "mlir/IR/SymbolInterfaces.td"
include "mlir/IR/BuiltinAttributeInterfaces.td"
include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"

class BIR_Op<string mnemonic, list<Trait> traits = []>
    : Op<BIR_Dialect, mnemonic, traits>;

def BIR_AnyType : AnyTypeOf<[BIR_IntType, BIR_FloatType, BIR_StringType]>;

def BIR_ConstantOp : BIR_Op<"constant", [
  AllTypesMatch<["value", "result"]>,
]> {
  let summary = "constant";

  let description = [{
    ```mlir
    %0 = bir.constant #bir.int<1> : !bir.int
    ```
  }];

  let arguments = (ins TypedAttrInterface:$value);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat = "$value attr-dict";

  let hasVerifier = 1;
}

def BIR_AddOp : BIR_Op<"add"> {
  let summary = "add";

  let description = [{
    ```mlir
    %0 = bir.constant #bir.int<4> : !bir.int
    %1 = bir.constant #bir.int<2> : !bir.int
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
    %0 = bir.constant #bir.int<4> : !bir.int
    %1 = bir.constant #bir.int<2> : !bir.int
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
    %0 = bir.constant #bir.int<4> : !bir.int
    %1 = bir.constant #bir.int<2> : !bir.int
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
    %0 = bir.constant #bir.int<4> : !bir.int
    %1 = bir.constant #bir.int<2> : !bir.int
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
    %0 = bir.constant #bir.int<4> : !bir.int
    %1 = bir.constant #bir.int<2> : !bir.int
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
    %0 = bir.constant #bir.int<0> : !bir.int
    bir.print %0 : !bir.int
    ```
  }];

  let arguments = (ins BIR_AnyType:$value);

  let assemblyFormat = "attr-dict $value `:` type($value)";
}

def BIR_FuncOp : BIR_Op<"func", [FunctionOpInterface, CallableOpInterface,
                             IsolatedFromAbove]> {
  let summary = "func";

  let arguments = (ins SymbolNameAttr:$sym_name,
      TypeAttrOf<FunctionType>:$function_type,
      OptionalAttr<DictArrayAttr>:$arg_attrs,
      OptionalAttr<DictArrayAttr>:$res_attrs);
  let regions = (region AnyRegion:$body);

  let builders = [OpBuilder<(ins "::llvm::StringRef":$name,
      "::mlir::FunctionType":$type,
      CArg<"::llvm::ArrayRef<::mlir::NamedAttribute>", "{}">:$attrs)>];

  let extraClassDeclaration = [{
    // -------------------------------------------------------------------------
    // FunctionOpInterface
    // -------------------------------------------------------------------------

    ::mlir::Region *getCallableRegion() { return &getBody(); }

    ArrayRef<::mlir::Type> getArgumentTypes() { return getFunctionType().getInputs(); }

    ArrayRef<::mlir::Type> getResultTypes() { return getFunctionType().getResults(); }

    mlir::Type getResType();
  }];

  let hasCustomAssemblyFormat = 1;
}

def BIR_FuncExprOp : BIR_Op<"func_expr", [
  // TODO: support closure variable captures
  IsolatedFromAbove,
  SingleBlock,
  SingleBlockImplicitTerminator<"bir::ReturnOp">,
]> {
  let summary = "func_expr";

  let arguments = (ins);

  let regions = (region AnyRegion:$body);

  let results = (outs FunctionType:$result);

  let assemblyFormat = "attr-dict `:` type($result) $body";
  let hasVerifier = 1;
}

def BIR_CallOp
    : BIR_Op<"call", [DeclareOpInterfaceMethods<CallOpInterface>,
                      DeclareOpInterfaceMethods<SymbolUserOpInterface>]> {
  let summary = "call";

  let arguments = (ins FlatSymbolRefAttr:$callee, Variadic<BIR_AnyType>:$args,
      OptionalAttr<DictArrayAttr>:$arg_attrs,
      OptionalAttr<DictArrayAttr>:$res_attrs);

  let results = (outs Optional<BIR_AnyType>:$result);

  let builders = [OpBuilder<(ins "mlir::SymbolRefAttr":$callee,
                      "mlir::Type":$resType, "mlir::ValueRange":$operands)>,
                  OpBuilder<(ins "bir::FuncOp":$funcOp,
                      "mlir::ValueRange":$operands)>,
  ];

  let skipDefaultBuilders = 1;
  let hasCustomAssemblyFormat = 1;
}

def BIR_CallIndirectOp : BIR_Op<"call_indirect", [
  CallOpInterface,
  TypesMatchWith<
    "callee input types match argument types",
    "callee", "callee_operands",
    "::llvm::cast<FunctionType>($_self).getInputs()"
  >,
  TypesMatchWith<
    "callee result types match result types",
    "callee", "results",
    "::llvm::cast<FunctionType>($_self).getResults()"
  >
]> {
  let summary = "call_indirect";

  let arguments = (ins
    FunctionType:$callee,
    Variadic<AnyType>:$callee_operands,
    OptionalAttr<DictArrayAttr>:$arg_attrs,
    OptionalAttr<DictArrayAttr>:$res_attrs
  );

  let results = (outs Variadic<AnyType>:$results);

  let assemblyFormat = "$callee `(` $callee_operands `)` attr-dict `:` type($callee)";

  let extraClassDeclaration = [{
    operand_range getArgOperands() {
      return {operand_begin(), operand_end()};
    }

    MutableOperandRange getArgOperandsMutable() {
      return getCalleeOperandsMutable();
    }

    CallInterfaceCallable getCallableForCallee() {
      return getCallee();
    }

    void setCalleeFromCallable(CallInterfaceCallable callee) {
      setOperand(0, cast<mlir::Value>(callee));
    }
  }];
}

def BIR_ReturnOp : BIR_Op<"return", [
  ParentOneOf<["FuncOp", "FuncExprOp"]>,
  Terminator,
]> {
  let summary = "return";

  let arguments = (ins Variadic<BIR_AnyType>:$input);

  let builders = [
    OpBuilder<(ins), [{ }]>,
  ];

  let assemblyFormat = "($input^ `:` type($input))? attr-dict";
}

def BIR_VarDeclareOp : BIR_Op<"var.declare"> {
  let summary = "var.declare";

  let arguments = (ins StrAttr:$name);
  let results = (outs BIR_RefType:$result);

  let assemblyFormat = "$name attr-dict `:` type($result)";
}

def BIR_VarStoreOp : BIR_Op<"var.store"> {
  let summary = "var.store";

  let arguments = (ins BIR_AnyType:$src, BIR_RefType:$dest);

  let assemblyFormat = "$src `to` $dest attr-dict `:` type($src) `to` type($dest)";
}

def BIR_VarLoadOp : BIR_Op<"var.load"> {
  let summary = "var.load";

  let arguments = (ins BIR_RefType:$ref);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat = "$ref attr-dict `:` functional-type(operands, results)";
}

#endif // BELALANG_IR_OPS_TD_
