// dix
#include <Pipeline/ShaderModule/ShaderModule.hpp>

// std
#include <fstream>
#include <stdexcept>

namespace dix {

std::vector<char> ShaderModule::readFile(const std::string& filepath) const {
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
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

ShaderModule::ShaderModule(VkDevice device, const std::string& spirvFilepath) : device_(device) {
    auto code = readFile(spirvFilepath);
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(device_, &createInfo, nullptr, &module_) != VK_SUCCESS) {
        module_ = VK_NULL_HANDLE;
    }
}

ShaderModule::~ShaderModule() {
    if (module_ != VK_NULL_HANDLE && device_ != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device_, module_, nullptr);
    }
}

} // namespace dix
