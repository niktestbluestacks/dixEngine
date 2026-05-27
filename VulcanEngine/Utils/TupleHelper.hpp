#ifndef TUPLE_HELPER_HPP
#define TUPLE_HELPER_HPP

#include <tuple>

namespace dix {

template <typename Tuple, typename Func>
void forEachInTuple(Tuple& tupleT, Func&& func) {
    std::apply([&](auto&&... args) { (func(args), ...); }, tupleT);
}

}  // namespace dix

#endif  // TUPLE_HELPER_HPP