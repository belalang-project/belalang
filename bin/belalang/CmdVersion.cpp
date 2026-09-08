#include "Belalang.h"
#include "belalang/Version.h"
#include "llvm/Support/raw_ostream.h"

namespace belalang {
namespace cmd {

int version() {
  llvm::outs() << "belalang commit " << getBelalangVersion() << "\n";
  return 0;
}

} // namespace cmd
} // namespace belalang
