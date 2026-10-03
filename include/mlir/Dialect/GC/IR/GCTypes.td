#ifndef MLIR_DIALECT_GC_IR_GCTYPES_TD_
#define MLIR_DIALECT_GC_IR_GCTYPES_TD_

include "mlir/Dialect/GC/IR/GCDialect.td"
include "mlir/Interfaces/DataLayoutInterfaces.td"

class GC_Type<string name, string typeMnemonic,
              list<Trait> traits = []>
    : TypeDef<GC_Dialect, name, traits> {
  let mnemonic = typeMnemonic;
}

def GC_PtrType : GC_Type<"Ptr", "ptr", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>
]> {
  let summary = "ptr";

  let parameters = (ins
    "::mlir::Type":$pointee
  );

  let assemblyFormat = "`<` $pointee `>`";
}

def GC_DerivedPtrType : GC_Type<"DerivedPtr", "derived_ptr", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>
]> {
  let summary = "derived_ptr";

  let parameters = (ins
    "::mlir::Type":$pointee);

  let assemblyFormat = "`<` $pointee `>`";
}

#endif // MLIR_DIALECT_GC_IR_GCTYPES_TD_
