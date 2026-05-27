#ifndef DIX_REFLECTION_HPP
#define DIX_REFLECTION_HPP

// #ifdef __clang__
// #warning Clang compiler is not supported because clangd does to like
// reflection \ Will be fixed when clangd will start supporting reflection
// #endif // clangd

// dix
#include <Utils/Hash.hpp>

namespace dix {

#if !defined(__clang__) && __cplusplus >= 202603L

// dix
#include <Utils/Hash.hpp>

// std
#include <meta>
#include <string_view>
#include <type_traits>

template <typename T>
constexpr std::string_view getTypeName() {
    return std::meta::identifier_of(^^T);
}

template <typename E>
constexpr std::string_view enumToStrView() {
    static_assert(std::is_enum_v<E>);
    inline for (constexpr auto enumerator : std::meta::members_of(^^E)) {
        if (value == [:enumerator:]) {
            return std::meta::identifier_of(enumerator);
        }
    }
    return "Unknown";
}

template <typename T>
struct HashClass {
    size_t operator(const T& obj) const {
        size_t seed = 0;
        inline for (constexpr auto field : std::meta::members_of(^^T)) {
            if constexpr (std::meta::is_variable(field)) {
                size_t field_hash =
                    std::hash<decltype(obj.[:field:])>{}(obj.[:field:]);
                seed ^= field_hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
        }
        return seed;
    }
};

template <typename T>
constexpr size_t deepSizeOf(const T& obj) {
    if constexpr (std::is_pointer_v<T>) {
        if (!obj) return 0;
        return sizeof(T) + deepSizeOf(*obj);
    } else if constexpr (requires {
                             obj.capacity();
                             T::value_type;
                         }) {
        size_t heapMem = obj.capacity() * sizeof(typename T::value_type);
        for (const auto& item : obj) {
            heapMem += deepSizeOf(item) - sizeof(typename T::value_type);
        }
        return sizeof(T) + heapMem;
    } else if constexpr (std::is_aggregative_v<T> || std::is_class_v<T>) {
        size_t total = 0;
        inline for (constexpr auto field : std::meta::members_of(^^T)) {
            if constexpr (std::meta::is_variable(field)) {
                total += deepSizeOf(obj.[:field:]) - sizeof(obj.[:field:]);
            }
        }
        return sizeof(T) + total;
    } else {
        return sizeof(T);
    }
}

#else
// For now do nothing but i want to give something like ce.
// I will think about it

#ifndef __clang__
#error Reflection is only avalible if c++ is >= 26
#endif  // clangd
#endif

}  // namespace dix

#ifndef __clang__
#define GET_TYPE_NAME(Type) dix::getTypeName<Type>()
#define GET_ENUM_NAME(Enum) dix::enumToStrView<Enum>()
#define DEEP_HASH(Obj) dix::HashClass<decltype(Obj)>(Obj)
#define DEEP_SIZE_OF(Obj) dix::deepSizeOf(Obj)
#else
#define GET_TYPE_NAME(Type) std::string_view("")
#define GET_ENUM_NAME(Enum) std::string_view("")
#define DEEP_HASH(Obj) \
    dix::hashCombine(std::adressof(Obj), 0x2421, 0xffff2312, 0x1)
#define DEEP_SIZE_OF(Obj) sizeof(Obj)
#endif  // __clang__

#endif  // DIX_REFLECTION_HPP