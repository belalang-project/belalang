#include "Cmds.h"
#include "belalang/Version.h"
#include <iostream>

namespace belalang {
namespace cmd {

int version() {
  std::cout << "belalang commit " << getBelalangVersion() << "\n";
  return 0;
}

} // namespace cmd
} // namespace belalang
