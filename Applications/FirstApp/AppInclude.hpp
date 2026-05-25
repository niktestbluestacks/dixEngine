#ifndef APP_INCLUDE_HPP
#define APP_INCLUDE_HPP

// dix
// LOGGING
#include <Logger/Logger.hpp>
#include <FrameRecorder/FrameRecorder.hpp>

// PIPELINE
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>

// RENDERING
#include <Rendering/Renderer/Renderer.hpp>
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>
#include <Rendering/RenderSystem/ParticleRenderSystem/ParticleRenderSystem.hpp>
#include <Rendering/RenderSystem/BoucyParticleRenderSystem/BouncyParticleRenderSystem.hpp>
#include <Rendering/RenderSystem/SkyboxRenderSystem/SkyboxRenderSystem.hpp>
// #include <Rendering/RenderSystem/ShadowMappingRenderSystem/ShadowMappingRenderSystem.hpp>
#include <Rendering/RenderSystem/RenderSystemRegistery.hpp>

// UI
#include <UI/UIManager.hpp>
#include <UI/UIRenderer.hpp>
#include <UI/DixUIElement.hpp>

// DIXUI
#include <DixUI/DixFpsCounter.hpp>
#include <DixUI/DixTimeCounter.hpp>
#include <DixUI/DixPlayerInfo.hpp>

// MODELING AND GAME OBJECTS
#include <Model/GameObject/GameObject.hpp>
#include <Model/DixTexture/DixTexture.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <DixCamera/DixCamera.hpp>
#include <Sound/DixAudio.hpp>

// WINDOW AND INPUT
#include <Window/WindowClass/WindowClass.hpp>
#include <Input/Keyboard/KeyboardAndMouseController.hpp>

// UTILS
#include <Utils/Converter.hpp>
#include <Utils/TupleHelper.hpp>
#include <Utils/DixRandom.hpp>
#include <Utils/FrameInfo.hpp>
#include <Utils/DixConcepts.hpp>
// dix

// libs
// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// VULKAN
#include <vulkan/vulkan.hpp>
// libs

// std
#if defined(__GNUG__) && __cplusplus > 202302L
#include <bits/stdc++.h>    // including all of the standtard library won't hurt the performance, it will just make life easier
#else
#include <chrono>
#include <string>
#include <filesystem>
#include <random>
#include <vector>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <utility>
#endif // __GNUG__
// std

#endif // APP_INCLUDE_HPP