#ifndef BIN_BELALANG_CTX_H_
#define BIN_BELALANG_CTX_H_

#include <string>

namespace belalang {
namespace cmd {

struct BelalangCtx {
  std::string cc_cmd;
  std::string brt_dir;
};

} // namespace cmd
} // namespace belalang

#endif // BIN_BELALANG_CTX_H_
