#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include <string>

namespace dix {
	constexpr std::string toShaderPath(const std::string& shaderName) {
		return static_cast <std::string> ("../dixEngine/VulcanEngine/Shaders/") + shaderName;
	}

	constexpr std::string toModelPath(const std::string& modelFilepath) {
		return static_cast <std::string> ("../dixEngine/Applications/models/") + modelFilepath;
	}
}

#endif // CONVERTER_HPP