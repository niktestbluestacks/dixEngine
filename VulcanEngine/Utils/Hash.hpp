#ifndef _HASH_HPP_
#define _HASH_HPP_

// std
#include <functional>

namespace dix {

template <typename T, typename ...Rest>
void hashCombine(std::size_t& seed, const T& v, const Rest&... rest) {
	seed ^= std::hash <T>{}(v)+0x9e3779b9u + (seed << 6) + (seed >> 2);
	(hashCombine(seed, rest), ...);
}

}	// namespace dix

#endif // _HASH_HPP_