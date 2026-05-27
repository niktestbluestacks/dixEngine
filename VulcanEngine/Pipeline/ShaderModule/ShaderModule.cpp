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

ShaderModule::ShaderModule(vk::Device device, const std::string& spirvFilepath) : device_(device) {
    auto code = readFile(spirvFilepath);
    vk::ShaderModuleCreateInfo createInfo{};
    createInfo.setCodeSize(code.size());
    createInfo.setPCode(reinterpret_cast<const uint32_t*>(code.data()));

    try {
        module_ = device_.createShaderModule(createInfo);
    } catch (...) {
        module_ = nullptr;
    }
}

ShaderModule::~ShaderModule() {
    if (module_ != nullptr && device_ != nullptr) {
        device_.destroyShaderModule(module_);
    }
}

} // namespace dix