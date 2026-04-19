// dix
#include <FirstApp/FirstApp.hpp>

#include <DixCamera/DixCamera.hpp>
#include <Pipeline/Buffer/DixBuffer.hpp>
#include <Input/Keyboard/KeyboardController.hpp>
#include <Rendering/RenderSystem/SimpleRenderSystem/SimpleRenderSystem.hpp>
#include <Utils/Converter.hpp>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cstdint>
#include <chrono>
#include <stdexcept>

namespace dix {

struct GlobalUbo {
	alignas(16) glm::mat4 projectionView{ 1.f };
	alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.f, -3.f, -1.f });
};

FirstApp::FirstApp() {
	Logger::get().log(Logger::INFO, "Initiating the application...");
	Logger::get().log(Logger::INFO, "-----------------------------");


	globalPool = DixDescriptorPool::Builder(m_dixDevice)
		.setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
		.build();

	loadGameObjects();
	
	
	
	Logger::get().log(Logger::INFO, "Application initialized succsesfully!");
}

FirstApp::~FirstApp() {
	Logger::get().log(Logger::INFO, "Closing the application...");
	Logger::get().log(Logger::INFO, "--------------------------");
	Logger::get().log(Logger::INFO, "Application has been closed succsesfully!");
}

void FirstApp::run(void) {
	std::vector <std::unique_ptr<DixBuffer>> uboBuffers{SwapChain::MAX_FRAMES_IN_FLIGHT};

	for (auto& uboBuffer : uboBuffers) {
		uboBuffer = std::make_unique <DixBuffer> (
			m_dixDevice,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
		);

		uboBuffer->map();
	}

	auto globalSetLayout = DixDescriptorSetLayout::Builder(m_dixDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
		.build();

	std::vector <VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
	for (size_t i = 0, globalDescriptorSetsSize = globalDescriptorSets.size(); i < globalDescriptorSetsSize; ++i) {
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		DixDescriptorWriter(*globalSetLayout, *globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
	}

	SimpleRenderSystem simpleRenderSystem { 
		m_dixDevice, 
		m_dixRenderer.getSwapChainRenderPass(), 
		globalSetLayout->getDescriptorSetLayout()
	};

    DixCamera dixcamera{};

    dixcamera.setViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});

    auto viewerObject = GameObject::createGameObject();
    KeyboardController cameraController{};

    auto currentTime = std::chrono::high_resolution_clock::now();

	while (!m_Window.shouldClose()) {
		glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration <float, std::chrono::seconds::period> (newTime - currentTime).count();
        currentTime = newTime;

        frameTime = glm::min(frameTime, MAX_FRAME_TIME);

        cameraController.modeInPlaneXZ(m_Window.getGLFWwindow(), frameTime, viewerObject);
        dixcamera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

        float aspect = m_dixRenderer.getAspectRatio();
        dixcamera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 10.f);
        if (auto commandBuffer = m_dixRenderer.beginFrame()) {
			int frameIndex = m_dixRenderer.getFrameIndex();
			FrameInfo frameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				dixcamera,
				globalDescriptorSets[frameIndex]
			};
			// update
			GlobalUbo ubo{};
			ubo.projectionView = dixcamera.getProjection() * dixcamera.getView();
			uboBuffers[frameIndex]->writeToIndex(&ubo, frameIndex);
			uboBuffers[frameIndex]->flush();
			// render
			m_dixRenderer.beginSwapChainRenderPass(commandBuffer);
			simpleRenderSystem.renderGameObjects(frameInfo, m_gameObjects);
			m_dixRenderer.endSwapChainRenderPass(commandBuffer);
			m_dixRenderer.endFrame();
		}
	}

	vkDeviceWaitIdle(m_dixDevice.device());
}

void FirstApp::loadGameObjects() {
	std::shared_ptr <Model> dixModel = Model::createModelFromFile(m_dixDevice, toModelPath("flat_vase.obj"));

    auto gameObj = GameObject::createGameObject();
    gameObj.model = dixModel;
    gameObj.transform.translation = { .0f, .5f, 2.5f };
	gameObj.transform.scale = { 3.f, 1.5f, 3.f };
    m_gameObjects.push_back(std::move(gameObj));
}

} // namespace dix