#ifndef DIX_CONCEPTS_HPP
#define DIX_CONCEPTS_HPP

// std
#include <concepts>
#include <tuple>

namespace dix {

template <typename T>
concept HasUbos = requires (T t) {
    { T::Ubos };
};

template <typename T>
concept HasName = requires (T t) {
    { T::Name() };
};

template <typename T>
struct is_tuple : std::false_type {};

// Specialization: match any std::tuple
template <typename... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_tuple_v = is_tuple<T>::value;

template <typename T>
concept HasVulkanFlags = requires (T t) {
    { T::getVulkanFlags };
};

}   // namespace dix

#endif // DIX_CONCEPTRS_HPP