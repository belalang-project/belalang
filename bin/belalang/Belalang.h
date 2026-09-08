#ifndef BIN_BELALANG_BELALANG_H_
#define BIN_BELALANG_BELALANG_H_

#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Error.h"
#include "llvm/Support/raw_ostream.h"
#include "muopt/muopt.hpp"
#include <string>

namespace belalang {
namespace cmd {

struct BelalangCtx {
  std::string cc_cmd;
  std::string brt_dir;
};

using Path = llvm::SmallString<128>;

llvm::Expected<Path> makeTempDir(llvm::StringRef prefix);
void removeTempDir(llvm::StringRef directory);

Path pathIn(llvm::StringRef directory, llvm::StringRef filename);
Path executablePathFor(llvm::StringRef source);

llvm::Expected<int> link(const BelalangCtx &ctx, llvm::StringRef objectFile,
                         llvm::StringRef executable);
llvm::Expected<int> execute(llvm::StringRef executable);

int build(muopt::Parser &, const BelalangCtx &);
int run(muopt::Parser &, const BelalangCtx &);
int version();

namespace term {

llvm::raw_ostream &error();
llvm::raw_ostream &warning();
llvm::raw_ostream &hint();

} // namespace term
} // namespace cmd
} // namespace belalang

#endif // BIN_BELALANG_BELALANG_H_
