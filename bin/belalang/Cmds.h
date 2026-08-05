#ifndef BIN_BELALANG_COMMANDS_H_
#define BIN_BELALANG_COMMANDS_H_

#include "muopt/muopt.hpp"

namespace belalang {
namespace cmd {

int build(muopt::Parser &);
int run(muopt::Parser &);
int version();

} // namespace cmd
} // namespace belalang

#endif // BIN_BELALANG_COMMANDS_H_
