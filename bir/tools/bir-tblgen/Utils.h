#include "mlir/TableGen/Operator.h"

class OpMetadata {
public:
  OpMetadata(mlir::tblgen::Operator &op) : op(op) {
    for (auto region : op.getRegions()) {
      auto regionName = region.name.str();
      regionNames.push_back(regionName);
    }
  };

  bool requiresGuard() { return op.getNumRegions() > 0; }

  mlir::tblgen::Operator getOp() { return this->op; }

  std::vector<std::string> getRegionNames() { return regionNames; };

  std::string getOpIdent() { return op.getCppClassName().str(); };
  std::string getFullOpIdent() {
    return op.getCppNamespace().str() + "::" + op.getCppClassName().str();
  }

  std::string getGuardName() {
    return "BIR" + op.getCppClassName().str() + "Guard";
  }

private:
  mlir::tblgen::Operator &op;
  std::vector<std::string> regionNames;
};
