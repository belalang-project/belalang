#include "belalang/BIR/IR/BIR.h"

#include "mlir/IR/DialectImplementation.h"

#define GET_TYPEDEF_CLASSES
#include "belalang/BIR/IR/BIRTypes.cpp.inc"

#define GET_ATTRDEF_CLASSES
#include "belalang/BIR/IR/BIRAttrs.cpp.inc"

#include "belalang/BIR/IR/BIRDialect.cpp.inc"
#include "belalang/BIR/IR/BIREnumAttrs.cpp.inc"

namespace {
using namespace belalang;

struct BIROpAsmDialectInterface : public OpAsmDialectInterface {
  using OpAsmDialectInterface::OpAsmDialectInterface;

  AliasResult getAlias(Type type, raw_ostream &os) const override {
    if (auto structType = dyn_cast<bir::StructType>(type)) {
      auto nameAttr = structType.getName();
      os << "struct_" << nameAttr.getValue();
      return AliasResult::OverridableAlias;
    }
    return AliasResult::NoAlias;
  }
};
} // namespace

namespace belalang {
namespace bir {

void BIRDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "belalang/BIR/IR/BIRTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "belalang/BIR/IR/BIROps.cpp.inc"
      >();

  addAttributes<
#define GET_ATTRDEF_LIST
#include "belalang/BIR/IR/BIRAttrs.cpp.inc"
      >();

  addInterface<BIROpAsmDialectInterface>();
}

} // namespace bir
} // namespace belalang
