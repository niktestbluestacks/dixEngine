#ifndef APP_INCLUDE_HPP
#define APP_INCLUDE_HPP

// dix
// LOGGING
#include <FrameRecorder/FrameRecorder.hpp>
#include <Logger/Logger.hpp>

// PIPELINE
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>

// RENDERING
#include <Rendering/RenderSystem/BoucyParticleRenderSystem/BouncyParticleRenderSystem.hpp>
#include <Rendering/RenderSystem/ParticleRenderSystem/ParticleRenderSystem.hpp>
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>
#include <Rendering/RenderSystem/SkyboxRenderSystem/SkyboxRenderSystem.hpp>
#include <Rendering/Renderer/Renderer.hpp>

// #include
// <Rendering/RenderSystem/ShadowMappingRenderSystem/ShadowMappingRenderSystem.hpp>
#include <Rendering/RenderSystem/RenderSystemRegistery.hpp>

// UI
#include <UI/DixUIElement.hpp>
#include <UI/UIManager.hpp>
#include <UI/UIRenderer.hpp>

// DIXUI
#include <DixUI/DixFpsCounter.hpp>
#include <DixUI/DixPlayerInfo.hpp>
#include <DixUI/DixTimeCounter.hpp>

// MODELING AND GAME OBJECTS
#include <DixCamera/DixCamera.hpp>
#include <Model/DixTexture/DixTexture.hpp>
#include <Model/GameObject/GameObject.hpp>
#include <Sound/DixAudio.hpp>

// WINDOW AND INPUT
#include <Input/Keyboard/KeyboardAndMouseController.hpp>
#include <Window/WindowClass/WindowClass.hpp>

// UTILS
#include <Utils/Converter.hpp>
#include <Utils/DixConcepts.hpp>
#include <Utils/DixRandom.hpp>
#include <Utils/FrameInfo.hpp>
#include <Utils/TupleHelper.hpp>

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
#include <bits/stdc++.h>  // including all of the standtard library won't hurt the performance, it will just make life easier
#else
#include <chrono>
#include <filesystem>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#endif  // __GNUG__
// std

#endif  // APP_INCLUDE_HPP