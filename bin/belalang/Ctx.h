#ifndef BIN_BELALANG_CTX_H_
#define BIN_BELALANG_CTX_H_

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include <string>

namespace belalang {
namespace cmd {

struct BelalangCtx {
  std::string cc_cmd;
  std::string brt_dir;
};

using Path = llvm::SmallString<128>;

llvm::Expected<Path> createTemporaryDirectory(llvm::StringRef prefix);
void removeTemporaryDirectory(llvm::StringRef directory);

Path pathInDirectory(llvm::StringRef directory, llvm::StringRef filename);
Path executablePathForSource(llvm::StringRef source);

llvm::Expected<int> link(const BelalangCtx &ctx, llvm::StringRef objectFile,
                         llvm::StringRef executable);
llvm::Expected<int> execute(llvm::StringRef executable);

} // namespace cmd
} // namespace belalang

#endif // BIN_BELALANG_CTX_H_
