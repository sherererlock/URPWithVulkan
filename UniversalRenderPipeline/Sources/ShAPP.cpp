
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
#include "camera.hpp"

#include "SimpleRenderSystem.h"
#include "GltfRenderSystem.h"
#include "PointLight.h"
#include "InputController.h"
#include "ShadowPass.h"
#include "ShadowRenderSystem.h"

#define MAX_SET_NUM 20
#define MAX_UNIFORM_BUFFER_NUM 20
#define MAX_SAMPLER_BUFFER_NUM 20

ShAPP::ShAPP() {
	globalPool =
		ShDescriptorPool::Builder(shDevice)
		.setMaxSets(MAX_SET_NUM)
		.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MAX_UNIFORM_BUFFER_NUM)
		.addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_SAMPLER_BUFFER_NUM)
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

	ShadowPass shadowPass{ shDevice,ShadowResolution, ShadowResolution };
	ShadowRenderSystem shadowRenderSystem{ shDevice, shadowPass.getRenderPass(), "shaders/spv/shadow_vert.hlsl.spv", "shaders/spv/shadow_frag.hlsl.spv", shadowPass.getShadowMapImageInfo()};
	shadowRenderSystem.setupDescriptorSet(*globalPool);

	std::vector<VkDescriptorSetLayout> setlayouts{ globalSetLayout->getDescriptorSetLayout()};

	//SimpleRenderSystem simpleRenderSystem{
	//	shDevice,
	//	shRenderer.getSwapChainRenderPass(),
	//	"shaders/spv/simple_shader.vert.spv", "shaders/spv/simple_shader.frag.spv", };

	//simpleRenderSystem.createPipelineLayout(setlayouts);
	//simpleRenderSystem.createPipeline(shRenderer.getSwapChainRenderPass());

	GltfRenderSystem gltfRenderSystem{ shDevice, shRenderer.getSwapChainRenderPass(), setlayouts, "shaders/spv/pbr_vert.hlsl.spv", "shaders/spv/pbr_frag.hlsl.spv", &shadowRenderSystem };

	PointLightSystem pointLightSystem{
		shDevice,
		shRenderer.getSwapChainRenderPass(),
		globalSetLayout->getDescriptorSetLayout() };

	Camera2 camera{};
	camera.type = Camera2::CameraType::firstperson;
	camera.flipY = true;
	camera.movementSpeed = 2.0f;
	camera.setPosition(glm::vec3(0.0f, 0.0f, -1.0f));
	camera.setRotation(glm::vec3(-0.0f, 0.0f, 0.0f));
	camera.setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 256.0f);

	Input input(camera, *this);

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

		camera.update(frameTime);
		input.update(frameTime);

		float aspect = shRenderer.getAspectRatio();

		if (auto commandBuffer = shRenderer.beginFrame())
		{
			int frameIndex = shRenderer.getFrameIndex();
			FrameInfo frameInfo{
				frameIndex,
				frameTime,
				lightUpdate,
				commandBuffer,
				camera,
				globalDescriptorSets[frameIndex],
				gameObjects };

			// update
			GlobalUbo ubo{};

			ubo.projection = camera.matrices.perspective;
			ubo.view = camera.matrices.view;
			ubo.inverseView = glm::inverse(camera.matrices.view);
			//ubo.viewPos = ubo.inverseView[3];
			ubo.viewPos = camera.viewPos;

			
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

	//const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices | vkglTF::FileLoadingFlags::FlipY;
	const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices;
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
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(0.8f, 1.0f, 2.413f, 1.f));
		gameObjects.emplace(pointLight.getId(), std::move(pointLight));
	}
}
