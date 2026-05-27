#ifndef DIX_DEBUG_HPP
#define DIX_DEBUG_HPP

#include <debugging>

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