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
