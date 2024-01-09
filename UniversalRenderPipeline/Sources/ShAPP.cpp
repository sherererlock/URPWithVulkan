
// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <windows.h>
// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/constants.hpp>

#include "ShAPP.h"

#include "shbuffer.h"
#include "camera.h"

#include "SimpleRenderSystem.h"
#include "GltfRenderSystem.h"
#include "PointLight.h"
#include "InputController.h"
#include "ShadowPass.h"
#include "ShadowRenderSystem.h"

ShAPP::ShAPP() {
	globalPool =
		ShDescriptorPool::Builder(shDevice)
		.setMaxSets(ShSwapchain::MAX_FRAMES_IN_FLIGHT * 2)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, ShSwapchain::MAX_FRAMES_IN_FLIGHT * 2)
		.build();
	loadGameObjects();
}

ShAPP::~ShAPP() {}

void ShAPP::run() 
{
	std::vector<std::unique_ptr<ShBuffer>> uboBuffers(ShSwapchain::MAX_FRAMES_IN_FLIGHT);

	for (int i = 0; i < uboBuffers.size(); i++)
	{
		uboBuffers[i] = std::make_unique<ShBuffer>(
			shDevice,
			sizeof(GlobalUbo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		uboBuffers[i]->map();
	}

	auto globalSetLayout =
		ShDescriptorSetLayout::Builder(shDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
		.build();

	std::vector<VkDescriptorSet> globalDescriptorSets(ShSwapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < globalDescriptorSets.size(); i++) {
		auto bufferInfo = uboBuffers[i]->descriptorInfo();
		ShDescriptorWriter(*globalSetLayout, *globalPool)
			.writeBuffer(0, &bufferInfo)
			.build(globalDescriptorSets[i]);
	}

	std::vector<VkDescriptorSetLayout> setlayouts{ globalSetLayout->getDescriptorSetLayout() };

	//SimpleRenderSystem simpleRenderSystem{
	//	shDevice,
	//	shRenderer.getSwapChainRenderPass(),
	//	"shaders/simple_shader.vert.spv", "shaders/simple_shader.frag.spv", };

	//simpleRenderSystem.createPipelineLayout(setlayouts);
	//simpleRenderSystem.createPipeline(shRenderer.getSwapChainRenderPass());

	GltfRenderSystem gltfRenderSystem{ shDevice, shRenderer.getSwapChainRenderPass(), setlayouts, "shaders/pbr_vert.hlsl.spv", "shaders/pbr_frag.hlsl.spv" };

	ShadowPass shadowPass{ shDevice,ShadowResolution, ShadowResolution };
	ShadowRenderSystem shadowRenderSystem{ shDevice, shadowPass.getRenderPass(), "shaders/shadow_vert.hlsl.spv", "shaders/shadow_frag.hlsl.spv"};
	shadowRenderSystem.setupDescriptorSet(*globalPool);

	PointLightSystem pointLightSystem{
		shDevice,
		shRenderer.getSwapChainRenderPass(),
		globalSetLayout->getDescriptorSetLayout() };
	Camera camera{};

	auto viewerObject = ShGameObject::createGameObject();
	viewerObject.transform.translation.z = -2.5f;
	viewerObject.transform.translation = glm::vec3(-0.104, -1.296f, 3.86f);
	viewerObject.transform.rotation = glm::vec3(-0.317f, 3.08f, 0.0f);
	KeyboardMovementController cameraController{};

	auto& pointLightGO = ShGameObject::getLight(gameObjects);
	auto currentTime = std::chrono::high_resolution_clock::now();
	while (!shWindow.shouldClose()) {
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		float frameTime =
			std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;
		DWORD sleeptime = (DWORD)(30.f - frameTime);
		Sleep(sleeptime);

		cameraController.moveInPlaneXZ(shWindow.getGLFWwindow(), frameTime, viewerObject);
		camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		float aspect = shRenderer.getAspectRatio();
		camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

		if (auto commandBuffer = shRenderer.beginFrame())
		{
			int frameIndex = shRenderer.getFrameIndex();
			FrameInfo frameInfo{
				frameIndex,
				frameTime,
				commandBuffer,
				camera,
				globalDescriptorSets[frameIndex],
				gameObjects };

			// update
			GlobalUbo ubo{};
			ubo.projection = camera.getProjection();
			ubo.view = camera.getView();
			ubo.inverseView = camera.getInverseView();
			pointLightSystem.update(frameInfo, ubo);
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			// shadow pass
			shadowRenderSystem.setupLight(pointLightGO, frameIndex);
			shadowPass.beginRenderPass(commandBuffer);
			shadowRenderSystem.renderGameObjects(frameInfo);
			shadowPass.endRenderPass(commandBuffer);

			// render
			shRenderer.beginSwapChainRenderPass(commandBuffer);

			// order here matters
			//simpleRenderSystem.renderGameObjects(frameInfo);
			gltfRenderSystem.renderGameObjects(frameInfo);

			pointLightSystem.render(frameInfo);

			shRenderer.endSwapChainRenderPass(commandBuffer);
			shRenderer.endFrame();
		}
	}

	vkDeviceWaitIdle(shDevice.device());
}

const std::string MODEL_PATH = "models/buster_drone/busterDrone.gltf";

void ShAPP::loadGameObjects() 
{
	//std::shared_ptr<ShModel> lveModel =
	//	ShModel::createModelFromFile(shDevice, "models/flat_vase.obj");
	//auto flatVase = ShGameObject::createGameObject();
	//flatVase.model = lveModel;
	//flatVase.transform.translation = { -.5f, .5f, 0.f };
	//flatVase.transform.scale = { 3.f, 1.5f, 3.f };
	//gameObjects.emplace(flatVase.getId(), std::move(flatVase));

	//lveModel = ShModel::createModelFromFile(shDevice, "models/smooth_vase.obj");
	//auto smoothVase = ShGameObject::createGameObject();
	//smoothVase.model = lveModel;
	//smoothVase.transform.translation = { .5f, .5f, 0.f };
	//smoothVase.transform.scale = { 3.f, 1.5f, 3.f };
	//gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

	//lveModel = ShModel::createModelFromFile(shDevice, "models/quad.obj");
	//auto floor = ShGameObject::createGameObject();
	//floor.model = lveModel;
	//floor.transform.translation = { 0.f, .5f, 0.f };
	//floor.transform.scale = { 3.f, 1.f, 3.f };
	//gameObjects.emplace(floor.getId(), std::move(floor));

	const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::PreMultiplyVertexColors | vkglTF::FileLoadingFlags::FlipY;
	std::shared_ptr<Model> gltfModel = std::make_shared<Model>();
	gltfModel->loadFromFile(MODEL_PATH, &shDevice, shDevice.graphicsQueue(), glTFLoadingFlags);

	auto gltfgo = ShGameObject::createGameObject();
	gltfgo.gltfmodel = gltfModel;
	gltfgo.transform.translation = { 0.f, -0.5f, 0.f };
	gltfgo.transform.scale = { 1.f, 1.f, 1.f };
	gameObjects.emplace(gltfgo.getId(), std::move(gltfgo));

	std::vector<glm::vec3> lightColors{
		{1.f, 1.f, 1.f},
		//{1.f, .1f, .1f},
		//{.1f, .1f, 1.f},
		//{.1f, 1.f, .1f},
		//{1.f, 1.f, .1f},
		//{.1f, 1.f, 1.f},
		//{1.f, 1.f, 1.f}  //
	};

	for (int i = 0; i < lightColors.size(); i++)
	{
		auto pointLight = ShGameObject::makePointLight(0.2f);
		pointLight.color = lightColors[i];
		auto rotateLight = glm::rotate(
			glm::mat4(1.f),
			(i * glm::two_pi<float>()) / lightColors.size(),
			{ 0.f, -1.f, 0.f });
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
		gameObjects.emplace(pointLight.getId(), std::move(pointLight));
	}
}
