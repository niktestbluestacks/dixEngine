#ifndef RENDER_SYSTEM_TRAITS_HPP
#define RENDER_SYSTEM_TRAITS_HPP

// dix
#include <Utils/DixConcepts.hpp>

// libs
#include <vulkan/vulkan.hpp>

// std
#include <cstdint>
#include <tuple>

#ifndef __clang__
#include <type_traits>
#include <concepts>
#endif // __clang_

namespace dix {

// Redeclared here for standalone use; identical to the declaration in
// DixRenderSystem.hpp.  C++ permits identical alias redeclarations.
using VulkanRenderSystemFlagType =
    std::tuple<uint32_t, vk::DescriptorType, vk::ShaderStageFlags>;

// Binding descriptor tags
//
// Use these as the elements of a render system's `Bindings` tuple.
// Each tag fully describes one VkDescriptorSetLayoutBinding slot in the
// per-render-system (set 0) descriptor set managed by AppContext.
//
// The type tags serve as a single source of truth from which both `VKBuffers`
// (the C++ types AppContext writes each frame) and `getVulkanFlags()` (the
// Vulkan binding metadata) are automatically derived.  Keeping them in sync
// by hand — and the resulting null-descriptor validation errors — is no
// longer possible.

// UniformBinding<UboT, Slot, Stages>
//
// A uniform-buffer binding whose contents are written every frame by
// AppContext from the render system's per-frame UBO buffer.
// UboT determines sizeof/alignof for the DixBuffer that is created, and is
// the type AppContext populates in drawFrame via if-constexpr field checks.
template <typename UboT, uint32_t Slot, vk::ShaderStageFlagBits Stages>
struct UniformBinding {
    using UboType = UboT;
    static constexpr uint32_t bindingIndex = Slot;
    static constexpr vk::ShaderStageFlagBits shaderStages = Stages;
    static constexpr vk::DescriptorType descriptorType =
        vk::DescriptorType::eUniformBuffer;
};

// SamplerBinding<Slot, Stages>
//
// A combined-image-sampler binding.  No associated C++ type — the texture is
// resolved at runtime and written into set 0 via AppContext's default-texture
// fallback (per-model textures live in set 1 via the model descriptor pool).
template <uint32_t Slot, vk::ShaderStageFlagBits Stages>
struct SamplerBinding {
    static constexpr uint32_t bindingIndex = Slot;
    static constexpr vk::ShaderStageFlagBits shaderStages = Stages;
    static constexpr vk::DescriptorType descriptorType =
        vk::DescriptorType::eCombinedImageSampler;
};

namespace detail {

// Concept: B has an associated UBO type — i.e. it is a buffer-type binding
// whose contents are managed by AppContext's per-frame UBO machinery.
template <typename B>
concept ManagedBuffer = requires { typename B::UboType; };

// Recursively collect UboType from a pack of binding tags, preserving
// declaration order and skipping non-buffer bindings (e.g. SamplerBinding).
template <typename... Bs>
struct ExtractUboTypes {
    using type = std::tuple<>;
};

template <ManagedBuffer B, typename... Rest>
struct ExtractUboTypes<B, Rest...> {
    using type = decltype(std::tuple_cat(
        std::declval<std::tuple<typename B::UboType>>(),
        std::declval<typename ExtractUboTypes<Rest...>::type>()));
};

template <typename B, typename... Rest>
    requires(!ManagedBuffer<B>)
struct ExtractUboTypes<B, Rest...> {
    using type = typename ExtractUboTypes<Rest...>::type;
};

}  // namespace detail

// RenderSystemTraits<BindingsTuple>
//
// Inherit from this mixin to get `VKBuffers` and `getVulkanFlags()` for free,
// guaranteed to stay in sync because both are derived from the same Bindings.
//
// Usage:
//
//   using Bindings = std::tuple<
//       UniformBinding<MyUbo, 0, VK_SHADER_STAGE_VERTEX_BIT>,
//       SamplerBinding<1,        VK_SHADER_STAGE_FRAGMENT_BIT>
//   >;
//
//   class MyRenderSystem
//       : public DixRenderSystem
//       , public RenderSystemTraits<Bindings>
//   {
//   public:
//       using PushConstantData = MyPushConstantData;
//       static constexpr const char* Name() { return "MyRenderSystem"; }
//       // ... constructor
//   };
//
// The `VKBuffers` and `getVulkanFlags()` required by RenderSystemRegistery and
// AppContext are provided automatically — no manual duplication needed.

template <typename BindingsTuple>
struct RenderSystemTraits;

template <typename... Bs>
struct RenderSystemTraits<std::tuple<Bs...>> {
    // Tuple of the C++ UBO types for every buffer-type binding, in
    // declaration order.  AppContext creates one DixBuffer per entry and
    // writes it into set 0 each frame via m_systemUboBuffers.
    using VKBuffers = typename detail::ExtractUboTypes<Bs...>::type;

    // Returns (bindingIndex, descriptorType, shaderStages) for every
    // binding in declaration order.  Satisfies the HasVulkanFlags concept
    // used by RenderSystemRegistery.
    static constexpr decltype(auto) getVulkanFlags() {
        return std::make_tuple(VulkanRenderSystemFlagType{
            Bs::bindingIndex, Bs::descriptorType, Bs::shaderStages}...);
    }
};

}  // namespace dix

#endif  // RENDER_SYSTEM_TRAITS_HPP