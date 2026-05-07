#ifndef SHADER_MODULE_HPP
#define SHADER_MODULE_HPP

#include <vulkan/vulkan.hpp>
#include <string>
#include <vector>
#include <memory>

namespace dix {

class ShaderModule {
public:
    ShaderModule() = default;
    ShaderModule(VkDevice device, const std::string& spirvFilepath);
    ~ShaderModule();

    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;

    VkShaderModule getModule() const { return module_; }
    bool isValid() const { return module_ != VK_NULL_HANDLE; }

private:
    std::vector<char> readFile(const std::string& filepath) const;

    VkDevice device_ = VK_NULL_HANDLE;
    VkShaderModule module_ = VK_NULL_HANDLE;
};

} // namespace dix

#endif // SHADER_MODULE_HPP
