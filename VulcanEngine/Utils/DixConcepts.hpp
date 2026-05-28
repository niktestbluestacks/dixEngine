#ifndef DIX_CONCEPTS_HPP
#define DIX_CONCEPTS_HPP

// std
#include <concepts>
#include <cstdint>
#include <tuple>
#include <type_traits>

// libs

#include <vulkan/vulkan.hpp>

namespace dix {

template <typename T>
concept HasUbos = requires { typename T::Ubos; };

template <typename T>
concept HasName = requires { T::Name(); };

template <typename T>
struct is_tuple : std::false_type {};

// Specialization: match any std::tuple
template <typename... Args>
struct is_tuple<std::tuple<Args...>> : std::true_type {};

template <typename T>
inline constexpr bool is_tuple_v = is_tuple<T>::value;

template <typename Expected, typename... Args>
concept AllOfTypesSame = (... && std::same_as<Args, Expected>);

template <typename T, typename Y>
struct same_tuple_type_as : std::false_type {};

template <typename T, typename... Args>
struct same_tuple_type_as<T, std::tuple<Args...>>
    : std::conditional_t<(... && std::same_as<T, Args>), std::true_type,
                         std::false_type> {};

template <typename T>
concept HasVulkanFlags = requires(T t) {
    { T::getVulkanFlags() };
    requires[]<typename... InnerTuples>(std::tuple<InnerTuples...>) {
        // Проверяем каждый внутренний элемент через свертку (fold expression)
        return (
            ... &&
            std::same_as<InnerTuples, std::tuple<uint32_t, vk::DescriptorType,
                                                 vk::ShaderStageFlags>>);
    }
    (decltype(T::getVulkanFlags()){});
};
}  // namespace dix

#endif  // DIX_CONCEPTRS_HPP