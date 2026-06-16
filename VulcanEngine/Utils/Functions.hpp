#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

// std
#include <concepts>
#include <format>
#include <string>
#include <variant>

namespace dix {
template <typename... Args>
constexpr std::string formatRuntime(const char* fmt, Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        return std::string(fmt);
    } else {
        // Has arguments: use vformat + make_format_args
        return std::vformat(fmt, std::make_format_args(args...));
    }
}

constexpr std::variant<std::string, int, float> string_to_num(
    const std::string& str) noexcept {
    if (str.empty()) return str;

    const char* first = str.data();
    const char* last = str.data() + str.size();

    int int_val{};
    auto [ptr_int, ec_int] = std::from_chars(first, last, int_val);
    if (ec_int == std::errc{} && ptr_int == last) {
        return int_val;
    }

    float float_val{};
    auto [ptr_float, ec_float] = std::from_chars(first, last, float_val);
    if (ec_float == std::errc{} && ptr_float == last) {
        return float_val;
    }

    return str;
}
}  // namespace dix
#endif  // FUNCTIONS_HPP