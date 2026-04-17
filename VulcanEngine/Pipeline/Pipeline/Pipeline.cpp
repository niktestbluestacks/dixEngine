#include <Pipeline/Pipeline/Pipeline.hpp>
#include <Logger/Logger.hpp>

#include <fstream>
#include <stdexcept>
//#include <filesystem>

namespace dix {

Pipeline::Pipeline(
	EngineDevice& device, 
	const std::string& vertFilepath, 
	const std::string& fragFilepath, 
	const PipelineConfigInfo& configInfo) : 
	dixdevice{device} {

	createGraphicsPipeline(vertFilepath, fragFilepath, configInfo);
}

PipelineConfigInfo Pipeline::defaultPipelineConfigInfo(uint32_t width, uint32_t height) {
	PipelineConfigInfo configInfo{};

	return configInfo;
}

std::vector<char> Pipeline::readFile(const std::string& filepath) {
	
	std::ifstream file{ filepath, std::ios::ate | std::ios::binary };

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file: " + filepath);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());

	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

void Pipeline::createGraphicsPipeline(
	const std::string& vertShaderCode, const std::string& fragShaderCode, const PipelineConfigInfo& configInfo) {

	auto vertCode = readFile(vertShaderCode);
	auto fragCode = readFile(fragShaderCode);

	Logger::get().log(Logger::DEBUG, "Vertex shader code size: " + std::to_string(vertCode.size()));
	Logger::get().log(Logger::DEBUG, "Fragment shader code size: " + std::to_string(fragCode.size()));
}

void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule) {
	VkShaderModuleCreateInfo createInfo {};

	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	if (vkCreateShaderModule(dixdevice.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}
}
} // namespace dix