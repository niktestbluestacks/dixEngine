#ifndef RENDER_SYSTEM_REGISTERY_HPP
#define RENDER_SYSTEM_REGISTERY_HPP

// dix
#include <Rendering/RenderSystem/DixRenderSystem.hpp>
#include <Utils/DixConcepts.hpp>
#include <memory>
#include <tuple>


namespace dix {
struct UboTypeInfo {
    size_t size;
    size_t alignment;
};

struct RenderSystemConstructInfo {
    EngineDevice& engineDevice;
    vk::RenderPass renderPass;
    vk::DescriptorSetLayout globalSetLayout;
    vk::DescriptorSetLayout modelSetLayout;
};

template <typename RenderSystem>
    requires HasUbos<RenderSystem> && is_tuple_v<typename RenderSystem::Ubos> &&
             HasName<RenderSystem> && HasVulkanFlags<RenderSystem>
struct RenderSystemDescription {
    std::unique_ptr<RenderSystem> renderSystem =
        nullptr;              // will be initialized later
    RenderSystem::Ubos Ubos;  // good already
    const char* renderSystemName = RenderSystem::Name();
};

template <typename... RenderSystems>
class RenderSystemRegistery {
   public:
    constexpr RenderSystemRegistery() = default;

    std::tuple<RenderSystemDescription<RenderSystems>...>&
    getRenderSystemDescriptions() {
        return m_renderSystems;
    }

    template <typename RenderSystem>
    constexpr decltype(auto) getRenderSystem() {
#ifndef __clang__
        constexpr bool systemExists = isRenderSystem<RenderSystem>();
        static_assert(systemExists, "That RenderSystem was not found");
#endif  // __clang__
        return *std::get<RenderSystemDescription<RenderSystem>>(m_renderSystems)
                    .renderSystem;
    }

    template <typename RenderSystem>
    consteval bool isRenderSystem() {
        return (std::is_same_v<RenderSystem, RenderSystems> || ...);
    }

   private:
    std::tuple<RenderSystemDescription<RenderSystems>...> m_renderSystems;
};
}  // namespace dix
#endif  // RENDER_SYSTEM_REGISTERY_HPP