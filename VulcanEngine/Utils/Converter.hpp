#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include <string>

namespace dix {
	constexpr std::string toShaderPath(const std::string& shaderName) {
		return static_cast <std::string> ("../dixEngine/VulcanEngine/Shaders/") + shaderName;
	}
}

#endif // CONVERTER_HPP