#ifndef BELALANG_BIR_INTERFACES_LOOPOPINTERFACE_TD_
#define BELALANG_BIR_INTERFACES_LOOPOPINTERFACE_TD_

include "mlir/IR/OpBase.td"
include "mlir/Interfaces/LoopLikeInterface.td"

def LoopOpInterface : OpInterface<"LoopOpInterface"> {
  let cppNamespace = "::belalang::bir";

  let methods = [
    InterfaceMethod<
      "returns the loop's conditional region",
      /*retTy = */"mlir::Region &",
      /*methodName = */"getCond"
    >,
    InterfaceMethod<
      "returns the loop's body region",
      /*retTy = */"mlir::Region &",
      /*methodName = */"getBody"
    >,
    InterfaceMethod<
      "returns the first region to be executed",
      /*retTy = */"mlir::Region &",
      /*methodName =*/"getEntry",
      /*args =*/(ins),
      /*methodBody =*/"",
      /*defaultImplementation =*/"return $_op.getCond();"
    >,
  ];
}

#endif // BELALANG_BIR_INTERFACES_LOOPOPINTERFACE_TD_
