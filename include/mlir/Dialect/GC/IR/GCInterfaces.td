#ifndef MLIR_DIALECT_GC_IR_GCINTERFACES_TD_
#define MLIR_DIALECT_GC_IR_GCINTERFACES_TD_

include "mlir/IR/OpBase.td"

def GC_SafepointOpInterface : OpInterface<"SafepointOpInterface"> {
  let cppNamespace = "::mlir::gc";

  let methods = [
    InterfaceMethod<
      /*desc=*/[{}],
      /*retTy=*/"::mlir::Operation::operand_range",
      /*methodName=*/"getRoots"
    >,
    InterfaceMethod<
      /*desc=*/[{}],
      /*retTy=*/"::mlir::Operation::result_range",
      /*methodName=*/"getRelocatedRoots"
    >,
    InterfaceMethod<
      /*desc=*/[{}],
      /*retTy=*/"::mlir::FailureOr<::mlir::gc::SafepointOpInterface>",
      /*methodName=*/"rebuildWithRoots",
      /*args=*/(ins
        "::mlir::RewriterBase &":$rewriter,
        "::mlir::ValueRange":$roots)
    >,
  ];
}

#endif
