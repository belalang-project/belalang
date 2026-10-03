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

  let description = [{
    `gc.ptr` represents a pointer that preserves the identity of an allocation.
    For a managed heap allocation, it points to the allocation base and may be
    used as a root at a GC safepoint. The collector may relocate such a root and
    produce an updated `gc.ptr` value.
  }];

  let parameters = (ins
    "::mlir::Type":$pointee
  );

  let assemblyFormat = "`<` $pointee `>`";
}

def GC_DerivedPtrType : GC_Type<"DerivedPtr", "derived_ptr", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>
]> {
  let summary = "derived_ptr";

  let description = [{
    `gc.derived_ptr` represents an address computed from a `gc.ptr` or another
    `gc.derived_ptr`, typically by `gc.ptr_offset`. It has the same target data
    layout as `gc.ptr`, but does not identify the base of an allocation and
    therefore cannot be used as a GC root.
  }];

  let parameters = (ins
    "::mlir::Type":$pointee);

  let assemblyFormat = "`<` $pointee `>`";
}

def GC_AnyPtrType : AnyTypeOf<[
  GC_PtrType,
  GC_DerivedPtrType,
]>;

#endif // MLIR_DIALECT_GC_IR_GCTYPES_TD_
