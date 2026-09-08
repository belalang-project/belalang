#include "Term.h"

namespace belalang {
namespace cmd {
namespace term {

llvm::raw_ostream &error() {
  llvm::raw_ostream &os = llvm::errs();
  os.changeColor(llvm::raw_ostream::RED, true);
  os << "error: ";
  os.resetColor();
  return os;
}

llvm::raw_ostream &warning() {
  llvm::raw_ostream &os = llvm::errs();
  os.changeColor(llvm::raw_ostream::YELLOW, true);
  os << "warning: ";
  os.resetColor();
  return os;
}

llvm::raw_ostream &hint() {
  llvm::raw_ostream &os = llvm::errs();
  os.changeColor(llvm::raw_ostream::CYAN, true);
  os << "hint: ";
  os.resetColor();
  return os;
}

} // namespace term
} // namespace cmd
} // namespace belalang
