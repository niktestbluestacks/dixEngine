#ifndef DIX_DEBUG_HPP
#define DIX_DEBUG_HPP

#ifndef __clang__
#include <debugging>
#endif // __clang__

namespace dix {

constexpr bool debugging() {
#ifdef NDEBUG
    return false;
#else
    return true;
#endif  // NDEBUG
}

}  // namespace dix

#endif  // DIX_DEBUG_HPP