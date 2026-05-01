#ifndef CONVERTER_HPP
#define CONVERTER_HPP

// std
#include <string>

namespace dix {
	constexpr std::string toShaderPath(const std::string& shaderName) {
		return std::string(static_cast <std::string> ("../dixEngine/build/shaders/") + shaderName);
	}

	constexpr std::string toModelPath(const std::string& modelFilepath) {
		return std::string(static_cast <std::string> ("../dixEngine/Applications/models/") + modelFilepath);
	}
}	// dix

#endif // CONVERTER_HPP