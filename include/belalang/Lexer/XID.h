#ifndef BELALANG_LEXER_XID_H_
#define BELALANG_LEXER_XID_H_

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <utility>

namespace belalang {
namespace lexer {

inline constexpr std::pair<char32_t, char32_t> XID_START[] = {
#define GEN_XID_START
#include "Unicode.h.inc"
};

inline constexpr std::pair<char32_t, char32_t> XID_CONTINUE[] = {
#define GEN_XID_CONTINUE
#include "Unicode.h.inc"
};

inline bool check_xid(const std::pair<char32_t, char32_t> *table, int size,
                      char32_t cp) {
  auto it = std::lower_bound(
      table, table + size, cp,
      [](const std::pair<char32_t, char32_t> &r, char32_t val) {
        {
          return r.second < val;
        }
      });
  return it != table + size && cp >= it->first;
}

inline bool is_xid_start(char32_t cp) {
  return check_xid(XID_START, std::size(XID_START), cp);
}

inline bool is_xid_continue(char32_t cp) {
  return check_xid(XID_CONTINUE, std::size(XID_CONTINUE), cp);
}

} // namespace lexer
} // namespace belalang

#endif // BELALANG_LEXER_XID_H_
