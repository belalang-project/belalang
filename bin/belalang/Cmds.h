#ifndef BIN_BELALANG_COMMANDS_H_
#define BIN_BELALANG_COMMANDS_H_

#include "Ctx.h"
#include "muopt/muopt.hpp"

namespace belalang {
namespace cmd {

int build(muopt::Parser &, const BelalangCtx &);
int run(muopt::Parser &, const BelalangCtx &);
int version();

} // namespace cmd
} // namespace belalang

#endif // BIN_BELALANG_COMMANDS_H_
