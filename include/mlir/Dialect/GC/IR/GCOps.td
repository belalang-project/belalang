#ifndef MLIR_DIALECT_GC_IR_GCOPS_TD_
#define MLIR_DIALECT_GC_IR_GCOPS_TD_

include "mlir/Dialect/GC/IR/GCDialect.td"
include "mlir/Dialect/GC/IR/GCTypes.td"

include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/MemorySlotInterfaces.td"

class GC_Op<string mnemonic, list<Trait> traits = []>
    : Op<GC_Dialect, mnemonic, traits> {}

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
    GC_PtrType:$result,
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
    GC_PtrType:$result
  );

  let assemblyFormat = [{
    `:` qualified(type($result)) attr-dict
  }];
}

#endif // MLIR_DIALECT_GC_IR_GCOPS_TD_
