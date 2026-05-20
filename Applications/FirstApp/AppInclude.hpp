#ifndef APP_INCLUDE_HPP
#define APP_INCLUDE_HPP

// dix
// LOGGING
#include <Logger/Logger.hpp>

// PIPELINE
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Pipeline/EngineDevice/EngineDevice.hpp>
#include <Pipeline/DixDescriptors/DixDescriptors.hpp>

// RENDERING
#include <Rendering/Renderer/Renderer.hpp>
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>
#include <Rendering/RenderSystem/ParticleRenderSystem/ParticleRenderSystem.hpp>
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

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan.hpp>

// std
#if defined(__GNUG__) && __GNUG__ >= 14
#include <bits/stdc++.h>
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

#endif // APP_INCLUDE_HPP