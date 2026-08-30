#ifndef BELALANG_IR_OPS_TD_
#define BELALANG_IR_OPS_TD_

include "belalang/BIR/IR/BIRDialect.td"
include "belalang/BIR/IR/BIRTypes.td"
include "belalang/BIR/IR/BIREnumAttrs.td"

include "belalang/BIR/Interfaces/LoopOpInterface.td"

include "mlir/IR/SymbolInterfaces.td"
include "mlir/IR/BuiltinAttributeInterfaces.td"
include "mlir/IR/EnumAttr.td"
include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/InferTypeOpInterface.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/MemorySlotInterfaces.td"

class BIR_Op<string mnemonic, list<Trait> traits = []>
    : Op<BIR_Dialect, mnemonic, traits> {}

def BIR_AnyType : AnyTypeOf<[
  BIR_IntType,
  BIR_FloatType,
  BIR_StringType,
  FunctionType,
  BIR_BoolType,
  BIR_StructType,
  BIR_ArrayType
]>;

def BIR_ConstantOp : BIR_Op<"constant", [
  AllTypesMatch<["value", "result"]>,
  Pure,
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

def BIR_AddOp : BIR_Op<"add", [
  SameTypeOperands,
  SameOperandsAndResultType,
  Pure
]> {
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

  let assemblyFormat = "$lhs `,` $rhs `:` type($lhs) attr-dict";
}

def BIR_SubOp : BIR_Op<"sub", [
  SameTypeOperands,
  SameOperandsAndResultType,
  Pure
]> {
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

  let assemblyFormat = "$lhs `,` $rhs `:` type($lhs) attr-dict";
}

def BIR_MulOp : BIR_Op<"mul", [
  SameTypeOperands,
  SameOperandsAndResultType,
  Pure
]> {
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

  let assemblyFormat = "$lhs `,` $rhs `:` type($lhs) attr-dict";
}

def BIR_DivOp : BIR_Op<"div", [
  SameTypeOperands,
  SameOperandsAndResultType,
  Pure
]> {
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

  let assemblyFormat = "$lhs `,` $rhs `:` type($lhs) attr-dict";
}

def BIR_ModOp : BIR_Op<"mod", [
  SameTypeOperands,
  SameOperandsAndResultType,
  Pure
]> {
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

  let assemblyFormat = "$lhs `,` $rhs `:` type($lhs) attr-dict";
}

def BIR_AndOp : BIR_Op<"and", [Pure]> {
  let summary = "and";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_OrOp : BIR_Op<"or", [Pure]> {
  let summary = "or";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_XorOp : BIR_Op<"xor", [Pure]> {
  let summary = "xor";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_ShlOp : BIR_Op<"shl", [Pure]> {
  let summary = "shl";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_ShrOp : BIR_Op<"shr", [Pure]> {
  let summary = "shr";

  let arguments = (ins BIR_AnyType:$lhs, BIR_AnyType:$rhs);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat =
      "$lhs `,` $rhs `:` functional-type(operands, results) attr-dict";
}

def BIR_FuncOp : BIR_Op<"func", [FunctionOpInterface, CallableOpInterface,
                             IsolatedFromAbove]> {
  let summary = "func";
  let hasVerifier = 1;

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

    ::llvm::ArrayRef<::mlir::Type> getArgumentTypes() { return getFunctionType().getInputs(); }

    ::llvm::ArrayRef<::mlir::Type> getResultTypes() { return getFunctionType().getResults(); }

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
    "::llvm::cast<::mlir::FunctionType>($_self).getInputs()"
  >,
  TypesMatchWith<
    "callee result types match result types",
    "callee", "results",
    "::llvm::cast<::mlir::FunctionType>($_self).getResults()"
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

    ::mlir::MutableOperandRange getArgOperandsMutable() {
      return getCalleeOperandsMutable();
    }

    ::mlir::CallInterfaceCallable getCallableForCallee() {
      return getCallee();
    }

    void setCalleeFromCallable(::mlir::CallInterfaceCallable callee) {
      setOperand(0, mlir::cast<mlir::Value>(callee));
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

def BIR_DeclareOp : BIR_Op<"declare"> {
  let summary = "declare";

  let arguments = (ins StrAttr:$name);
  let results = (outs BIR_RefType:$result);

  let assemblyFormat = "$name attr-dict `:` qualified(type($result))";
}

def BIR_StoreOp : BIR_Op<"store", [
  DeclareOpInterfaceMethods<PromotableMemOpInterface>
]> {
  let summary = "store";

  let arguments = (ins BIR_AnyType:$src, BIR_RefType:$dest);

  let assemblyFormat = "$src `to` $dest attr-dict `:` type($src) `to` qualified(type($dest))";
}

def BIR_LoadOp : BIR_Op<"load", [
  DeclareOpInterfaceMethods<PromotableMemOpInterface>
]> {
  let summary = "load";

  let arguments = (ins BIR_RefType:$ref);
  let results = (outs BIR_AnyType:$result);

  let assemblyFormat = "$ref attr-dict `:` functional-type(operands, results)";
}

def BIR_ScopeOp : BIR_Op<"scope", [
  NoRegionArguments,
]> {
  let regions = (region AnyRegion:$scopeRegion);
  let results = (outs Optional<BIR_AnyType>:$results);

  let assemblyFormat = "$scopeRegion (`:` type($results)^)? attr-dict";
}

def BIR_YieldOp : BIR_Op<"yield", [
  ParentOneOf<["ScopeOp", "IfOp", "WhileOp"]>,
  Terminator,
]> {
  let summary = "yield";

  let arguments = (ins Variadic<BIR_AnyType>:$args);
  let assemblyFormat = "($args^ `:` type($args))? attr-dict";

  let builders = [
    OpBuilder<(ins), [{  }]>,
  ];
}

def BIR_IfOp : BIR_Op<"if", [
  NoRegionArguments
]> {
  let summary = "if";

  let arguments = (ins BIR_BoolType:$cond);
  let regions = (region AnyRegion:$thenRegion, AnyRegion:$elseRegion);
  let results = (outs Optional<BIR_AnyType>:$result);

  let extraClassDeclaration = [{
    bool hasElse() { return !getElseRegion().empty(); }
    static ::mlir::LogicalResult ensureRegionTerm(::mlir::Builder &, ::mlir::Region &);
  }];

  let hasCustomAssemblyFormat = 1;
}

def BIR_CondBrOp : BIR_Op<"cond_br", [
  Terminator,
  AttrSizedOperandSegments,
]> {
  let summary = "cond_br";

  let builders = [
    OpBuilder<(
      ins "mlir::Value":$cond,
      "mlir::Block *":$destTrue,
      "mlir::Block *":$destFalse,
      CArg<"mlir::ValueRange", "{}">:$destOperandsTrue,
      CArg<"mlir::ValueRange", "{}">:$destOperandsFalse
    ), [{
      build($_builder, $_state, cond, destOperandsTrue,
            destOperandsFalse, destTrue, destFalse);
    }]>
  ];

  let arguments = (ins
    BIR_BoolType:$cond,
    Variadic<BIR_AnyType>:$destOperandsTrue,
    Variadic<BIR_AnyType>:$destOperandsFalse
  );

  let successors = (successor
    AnySuccessor:$destTrue,
    AnySuccessor:$destFalse
  );

  let assemblyFormat = [{
    $cond
    $destTrue (`(` $destOperandsTrue^ `:` type($destOperandsTrue) `)`)?
    `,`
    $destFalse (`(` $destOperandsFalse^ `:` type($destOperandsFalse) `)`)?
    attr-dict
  }];
}

// -----------------------------------------------------------------------------
// Loops
// -----------------------------------------------------------------------------

def BIR_WhileOp : BIR_Op<"while", [
  LoopOpInterface,
  NoRegionArguments,
]> {
  let regions = (region
    MinSizedRegion<1>:$cond,
    MinSizedRegion<1>:$body
  );
  
  let assemblyFormat = "$cond `do` $body attr-dict";
}

def BIR_ConditionOp : BIR_Op<"condition", [
  Terminator,
]> {
  let arguments = (ins BIR_BoolType:$cond);
  let assemblyFormat = "$cond attr-dict";
  let hasVerifier = 1;
}

def BIR_BreakOp : BIR_Op<"break", [
  Terminator,
  AncestorOneOf<["WhileOp"]>,
]> {
  let assemblyFormat = "attr-dict";
}

def BIR_ContinueOp : BIR_Op<"continue", [
  Terminator,
  AncestorOneOf<["WhileOp"]>,
]> {
  let assemblyFormat = "attr-dict";
}

def BIR_CmpOpKind : BIR_I32EnumAttr<"CmpOpKind", "compare operation kind", [
  I32EnumAttrCase<"lt", 0>,
  I32EnumAttrCase<"le", 1>,
  I32EnumAttrCase<"gt", 2>,
  I32EnumAttrCase<"ge", 3>,
  I32EnumAttrCase<"eq", 4>,
  I32EnumAttrCase<"ne", 5>
]>;

def BIR_ComparableType : AnyTypeOf<[
  BIR_IntType,
  BIR_FloatType
]> {}

def BIR_CmpOp : BIR_Op<"cmp", [
  Pure, SameTypeOperands
]> {
  let arguments = (ins
    BIR_CmpOpKind:$kind,
    BIR_ComparableType:$lhs,
    BIR_ComparableType:$rhs
  );
  let results = (outs BIR_BoolType:$result);
  let assemblyFormat = [{
    $kind $lhs `,` $rhs `:` type($lhs) attr-dict
  }];

}

def BIR_GetMemberOp : BIR_Op<"get_member", [Pure]> {
  let arguments = (ins
    BIR_RefType:$addr, // TODO: maybe make a pointer type?
    StrAttr:$name,
    IndexAttr:$index_attr
  );

  let results = (outs
    BIR_RefType:$result
  );

  let assemblyFormat = [{
    $addr `[` $index_attr `]` attr-dict
    `:` qualified(type($addr)) `->` qualified(type($result))
  }];
}

def BIR_AllocHeapOp : BIR_Op<"alloc_heap", [
  TypesMatchWith<"", "roots", "relocated_roots", "$_self">
]> {
  let arguments = (ins
    Variadic<BIR_RefType>:$roots
  );

  let results = (outs
    BIR_RefType:$result,
    Variadic<BIR_RefType>:$relocated_roots
  );

  let assemblyFormat = [{
    `:` qualified(type($result))
    (`roots` `(` $roots^ `:` type($roots) `)`)?
    attr-dict
  }];
}

def BIR_AllocStackOp : BIR_Op<"alloc_stack", [
  DeclareOpInterfaceMethods<PromotableAllocationOpInterface>
]> {
  let arguments = (ins);

  let results = (outs
    BIR_RefType:$result
  );

  let assemblyFormat = [{
    `:` qualified(type($result)) attr-dict
  }];
}

#endif // BELALANG_IR_OPS_TD_
