#ifndef MLIR_DIALECT_GC_IR_GCOPS_TD_
#define MLIR_DIALECT_GC_IR_GCOPS_TD_

include "mlir/Dialect/GC/IR/GCDialect.td"
include "mlir/Dialect/GC/IR/GCTypes.td"

include "mlir/Interfaces/MemorySlotInterfaces.td"

class GC_Op<string mnemonic, list<Trait> traits = []>
    : Op<GC_Dialect, mnemonic, traits> {}

def GC_AllocOp : GC_Op<"alloc", [
  TypesMatchWith<"", "roots", "relocated_roots", "$_self">
]> {
  let arguments = (ins
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
