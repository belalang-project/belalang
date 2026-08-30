#ifndef BELALANG_IR_TYPES_TD_
#define BELALANG_IR_TYPES_TD_

include "belalang/BIR/IR/BIRDialect.td"
include "mlir/Interfaces/DataLayoutInterfaces.td"

class BIR_Type<string name, string typeMnemonic, list<Trait> traits = [],
               string baseCppClass = "::mlir::Type">
    : TypeDef<BIR_Dialect, name, traits, baseCppClass> {
  let mnemonic = typeMnemonic;
}

def BIR_IntType : BIR_Type<"Int", "int", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>,
]> {
  let summary = "Integer number type";
}

def BIR_FloatType : BIR_Type<"Float", "float", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>,
]> {
  let summary = "Floating-point number type";
}

def BIR_StringType : BIR_Type<"String", "string", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>,
]> {
  let summary = "String type";
}

def BIR_BoolType : BIR_Type<"Bool", "bool", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>,
]> {
  let summary = "Boolean type";
}

def BIR_StructType : BIR_Type<"Struct", "struct", [
  MutableType,
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>,
]> {
  let summary = "Struct type";

  let storageClass = "StructTypeStorage";
  let genStorageClass = 0;

  let genVerifyDecl = 1;

  let parameters = (ins
    ArrayRefParameter<"mlir::Type", "struct members">:$members,
    "mlir::StringAttr":$name,
    ArrayRefParameter<"int32_t", "logical-to-physical field mapping">:$reorder
  );

  let hasCustomAssemblyFormat = 1;

  let extraClassDeclaration = [{
    ::mlir::LogicalResult setReorder(llvm::ArrayRef<int32_t> reorder);
    ::llvm::SmallVector<int32_t> getInverseReorder() const;
  }];

}

def BIR_ArrayType : BIR_Type<"Array", "array", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>,
]> {
  let summary = "Array type";

  let parameters = (ins
    ArrayRefParameter<"mlir::Type", "">:$members
  );

  let assemblyFormat = "`<` `[` $members `]` `>`";
}

def BIR_ReferencableType : AnyTypeOf<[
  BIR_IntType,
  BIR_FloatType,
  BIR_StringType,
  FunctionType,
  BIR_BoolType,
  BIR_StructType
]>;

def BIR_RefType : BIR_Type<"Ref", "ref", [
  DeclareTypeInterfaceMethods<DataLayoutTypeInterface>,
]> {
  let summary = "Reference type";

  let parameters = (ins BIR_ReferencableType:$referent);

  let assemblyFormat = "`<` $referent `>`";
}

#endif // BELALANG_IR_TYPES_TD_
