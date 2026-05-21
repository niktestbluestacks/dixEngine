#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

#include <string>
#include <format>
#include <concepts>

namespace dix {
template<typename... Args>
constexpr std::string formatRuntime(const char* fmt, Args&&... args) {
    if constexpr (sizeof...(args) == 0) {
        return std::string(fmt);
    } else {
        // Has arguments: use vformat + make_format_args
        return std::vformat(fmt, std::make_format_args(args...));
    }
}
}   // namespace dix
#endif // FUNCTIONS_HPP