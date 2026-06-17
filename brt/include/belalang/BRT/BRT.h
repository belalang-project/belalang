#ifndef BELALANG_BRT_BRT_H_
#define BELALANG_BRT_BRT_H_

#include <string_view>

namespace belalang {
namespace brt {

using namespace std::string_view_literals;

constexpr std::string_view BRT_PRINT_INT = "brt_print_int"sv;
constexpr std::string_view BRT_PRINT_FLOAT = "brt_print_float"sv;
constexpr std::string_view BRT_MMTK_ALLOC = "brt_mmtk_alloc"sv;
constexpr std::string_view BRT_MMTK_INIT = "brt_mmtk_init"sv;

} // namespace brt
} // namespace belalang

#endif // BELALANG_BRT_BRT_H_
