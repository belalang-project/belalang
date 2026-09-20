#ifndef MLIR_DIALECT_GC_IR_GCOPS_TD_
#define MLIR_DIALECT_GC_IR_GCOPS_TD_

include "mlir/Dialect/GC/IR/GCDialect.td"
include "mlir/Dialect/GC/IR/GCTypes.td"

include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/MemorySlotInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

class GC_Op<string mnemonic, list<Trait> traits = []>
    : Op<GC_Dialect, mnemonic, traits> {}

// TODO: Find a way to annotate call ops whether or not it has memory side
// effects.
def GC_CallOp : GC_Op<"call", [CallOpInterface]> {
  let arguments = (ins
    FlatSymbolRefAttr:$callee,
    Variadic<AnyType>:$operands,
    OptionalAttr<DictArrayAttr>:$arg_attrs,
    OptionalAttr<DictArrayAttr>:$res_attrs
  );

  let results = (outs Variadic<AnyType>);

  let builders = [
    OpBuilder<(ins
               "SymbolRefAttr":$callee,
               "TypeRange":$results,
               "ValueRange":$operands), [{
      $_state.addAttribute("callee", callee);
      $_state.addOperands(operands);
      $_state.addTypes(results);
    }]>
  ];

  let extraClassDeclaration = [{
    ::mlir::CallInterfaceCallable getCallableForCallee() {
      return getCalleeAttr();
    }

    void setCalleeFromCallable(::mlir::CallInterfaceCallable callee) {
      setCalleeAttr(::mlir::cast<::mlir::FlatSymbolRefAttr>(
          ::mlir::cast<::mlir::SymbolRefAttr>(callee)));
    }

    ::mlir::Operation::operand_range getArgOperands() {
      return getOperands();
    }

    ::mlir::MutableOperandRange getArgOperandsMutable() {
      return getOperandsMutable();
    }
  }];

  let assemblyFormat = [{
    $callee `(` $operands `)` attr-dict `:`
    functional-type($operands, results)
  }];
}

def GC_AllocOp : GC_Op<"alloc", [
  TypesMatchWith<"", "roots", "relocated_roots", "$_self">
]> {
  let builders = [
    OpBuilder<(ins "Type":$result), [{
      build($_builder,
            $_state,
            result,
            ::mlir::TypeRange{},
            ::mlir::IntegerAttr{},
            ::mlir::DenseI32ArrayAttr{},
            ::mlir::ValueRange{});
    }]>
  ];

  let arguments = (ins
    OptionalAttr<I64Attr>:$size,
    OptionalAttr<DenseI32ArrayAttr>:$pointer_offsets,
    Variadic<GC_PtrType>:$roots
  );

  let results = (outs
    Res<GC_PtrType, "",
        [MemAlloc<DefaultResource, 0, FullEffect>]>:$result,
    Variadic<GC_PtrType>:$relocated_roots
  );

  let assemblyFormat = [{
    (`roots` `(` $roots^ `:` type($roots) `)`)?
    `:` qualified(type($result))
    attr-dict
  }];
}

def GC_AllocaOp : GC_Op<"alloca", [
  DeclareOpInterfaceMethods<PromotableAllocationOpInterface>
]> {
  let arguments = (ins);

  let results = (outs
    Res<GC_PtrType, "",
        [MemAlloc<AutomaticAllocationScopeResource, 0, FullEffect>]>:$result
  );

  let assemblyFormat = [{
    `:` qualified(type($result)) attr-dict
  }];
}

def GC_LoadOp : GC_Op<"load", [
  TypesMatchWith<
    "",
    "address",
    "result",
    "::llvm::cast<::mlir::gc::PtrType>($_self).getPointee()"
  >
]> {
  let arguments = (ins
    Arg<GC_PtrType, "", [MemRead]>:$address
  );

  let results = (outs
    AnyType:$result
  );

  let assemblyFormat = [{
    $address attr-dict `:` qualified(type($address))
  }];
}

def GC_StoreOp : GC_Op<"store", [
  TypesMatchWith<
    "",
    "address",
    "value",
    "::llvm::cast<::mlir::gc::PtrType>($_self).getPointee()"
  >
]> {
  let arguments = (ins
    AnyType:$value,
    Arg<GC_PtrType, "", [MemWrite]>:$address
  );

  let results = (outs);

  let assemblyFormat = [{
    $value `,` $address
    attr-dict `:` qualified(type($address))
  }];
}

#endif // MLIR_DIALECT_GC_IR_GCOPS_TD_
