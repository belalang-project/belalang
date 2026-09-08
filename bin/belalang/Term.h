#ifndef BIN_BELALANG_TERM_H_
#define BIN_BELALANG_TERM_H_

#include "llvm/Support/raw_ostream.h"

namespace belalang {
namespace cmd {
namespace term {

llvm::raw_ostream &error();
llvm::raw_ostream &warning();
llvm::raw_ostream &hint();

} // namespace term
} // namespace cmd
} // namespace belalang

#endif // BIN_BELALANG_TERM_H_
