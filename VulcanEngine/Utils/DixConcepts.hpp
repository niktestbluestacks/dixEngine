#ifndef DIX_CONCEPTS_HPP
#define DIX_CONCEPTS_HPP

// dix
#include <Utils/DixMacroes.hpp>

// std
#include <concepts>
#include <cstdint>
#include <functional>
#include <tuple>
#include <type_traits>

// libs
#include <vulkan/vulkan.hpp>

namespace dix {

template <typename T>
concept HasVKBuffers = requires { typename T::VKBuffers; };

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
        return (
            ... &&
            std::same_as<InnerTuples, std::tuple<uint32_t, vk::DescriptorType,
                                                 vk::ShaderStageFlags>>);
    }
    (decltype(T::getVulkanFlags()){});
};

template <typename T>
struct is_supported_command_type : std::false_type {};

template <typename... FuncArgs>
struct is_supported_command_type<
    std::pair<std::string, std::function<void(FuncArgs...)>>> : std::true_type {
};

template <typename T>
inline constexpr bool is_supported_command_type_v =
    is_supported_command_type<T>::value;

template <typename... Commands>
concept IsSupportedCommandType = (is_supported_command_type_v<Commands> && ...);

}  // namespace dix

#define DIX_CREATE_IF_EXISTS(returnValue, methodName, Container, ...)       \
    constexpr returnValue methodName(                                       \
        __VA_OPT__(DIX_RECURSIVE_TO_TYPE_NAME(__VA_ARGS__)))                \
        requires requires(Container __c __VA_OPT__(                         \
            , DIX_RECURSIVE_TO_TYPE_NAME(__VA_ARGS__))) {                   \
            __c.methodName(__VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__))); \
        }

#define DIX_CREATE_IF_EXISTS_CONST(returnValue, methodName, Container, ...) \
    constexpr returnValue methodName(                                       \
        __VA_OPT__(DIX_RECURSIVE_TO_TYPE_NAME(__VA_ARGS__))) const          \
        requires requires(const Container __c __VA_OPT__(                         \
            , DIX_RECURSIVE_TO_TYPE_NAME(__VA_ARGS__))) {                   \
            __c.methodName(__VA_OPT__(DIX_RECURSIVE_TO_NAME(__VA_ARGS__))); \
        }

#endif  // DIX_CONCEPTS_HPP