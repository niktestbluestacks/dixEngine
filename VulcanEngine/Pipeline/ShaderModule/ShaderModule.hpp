#ifndef SHADER_MODULE_HPP
#define SHADER_MODULE_HPP

// libs
#include <vulkan/vulkan.hpp>

// std
#include <string>
#include <vector>
#include <memory>

namespace dix {

class ShaderModule {
public:
    ShaderModule() = default;
    ShaderModule(vk::Device device, const std::string& spirvFilepath);
    ~ShaderModule();

    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;

    vk::ShaderModule getModule() const { return module_; }
    bool isValid() const { return module_ != vk::ShaderModule{}; }

private:
    std::vector<char> readFile(const std::string& filepath) const;

    vk::Device device_ = nullptr;
    vk::ShaderModule module_ = nullptr;
};

} // namespace dix

#endif // SHADER_MODULE_HPP
