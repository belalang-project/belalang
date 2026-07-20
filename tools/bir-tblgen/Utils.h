#include "mlir/TableGen/Operator.h"
#include "llvm/Support/FormatVariadic.h"

inline std::string sc2cc(llvm::StringRef s) {
  return llvm::convertToCamelFromSnakeCase(s);
}

inline std::string cc2sc(llvm::StringRef s) {
  return llvm::convertToSnakeFromCamelCase(s);
}

static const char *const Banner = R"(
// --------------------------------------------------------------------------------
// {0}
// --------------------------------------------------------------------------------
)";

inline void emitCommentBanner(llvm::raw_ostream &os, llvm::Twine msg) {
  os << llvm::formatv(Banner, msg) << "\n";
}

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

  std::string getBuilderName() {
    return cc2sc("build" + op.getCppClassName().str());
  }

private:
  mlir::tblgen::Operator &op;
  std::vector<std::string> regionNames;
};
