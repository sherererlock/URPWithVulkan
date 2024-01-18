
// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>
#include <iostream>
#include <windows.h>


// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/constants.hpp>

#include "ShAPP.h"

#include "shbuffer.h"
#include "camera.hpp"

#include "RenderSystem/SimpleRenderSystem.h"
#include "RenderSystem/GltfRenderSystem.h"
#include "RenderSystem/PointLight.h"
#include "InputController.h"
#include "RenderPass/ShadowPass.h"
#include "RenderSystem/ShadowRenderSystem.h"
#include "RenderPass/BasePass.h"
#include "RenderPass/LightingPass.h"
#include "RenderPass/BlitPass.h"
#include "RenderSystem/BlitRenderSystem.h"

#include "macros.hlsl"

#define MAX_SET_NUM 20
#define MAX_UNIFORM_BUFFER_NUM 20
#define MAX_SAMPLER_BUFFER_NUM 20

#define DEFERRENDERING

struct CameraExtentUBO
{
	glm::vec4 leftTop;
	glm::vec4 left2Right;
	glm::vec4 top2bottom;
};

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

	BasePass basePass{ shDevice, WIDTH, HEIGHT };
	LightingPass lightPass{ shDevice, WIDTH, HEIGHT, shRenderer.getFormat()};

	std::vector<VkDescriptorSetLayout> setlayouts{ globalSetLayout->getDescriptorSetLayout()};

	//SimpleRenderSystem simpleRenderSystem{
	//	shDevice,
	//	shRenderer.getSwapChainRenderPass(),
	//	"shaders/spv/simple_shader.vert.spv", "shaders/spv/simple_shader.frag.spv", };

	//simpleRenderSystem.createPipelineLayout(setlayouts);
	//simpleRenderSystem.createPipeline(shRenderer.getSwapChainRenderPass());

#ifndef DEFERRENDERING
	GltfRenderSystem gltfRenderSystem{ shDevice, shRenderer.getSwapChainRenderPass(), setlayouts, "shaders/spv/pbr_vert.hlsl.spv", "shaders/spv/pbr_frag.hlsl.spv", &shadowRenderSystem };
#endif

#ifdef  CALC_POSITION
	GltfRenderSystem baseRenderSystem{ shDevice, basePass.getRenderPass(), setlayouts, "shaders/spv/gbuffer_skin.vert.spv", "shaders/spv/gbuffer_frag.hlsl.spv", nullptr, 3};
#else
	
#ifdef CPU_SKIN
	GltfRenderSystem baseRenderSystem{ shDevice, basePass.getRenderPass(), setlayouts, "shaders/spv/gbuffer_skin.vert.spv", "shaders/spv/gbuffer_frag.hlsl.spv", nullptr, 4 };
#elif defined CPU_ANIM
	GltfRenderSystem baseRenderSystem{ shDevice, basePass.getRenderPass(), setlayouts, "shaders/spv/gbuffer_anim.vert.spv", "shaders/spv/gbuffer_frag.hlsl.spv", nullptr, 4 };
#else
	GltfRenderSystem baseRenderSystem{ shDevice, basePass.getRenderPass(), setlayouts, "shaders/spv/gbuffer_vert.hlsl.spv", "shaders/spv/gbuffer_frag.hlsl.spv", nullptr, 4 };
#endif

#endif

	auto lightingSetLayout0 =
		ShDescriptorSetLayout::Builder(shDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

	auto lightingSetLayout1 =
		ShDescriptorSetLayout::Builder(shDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
#ifndef  CALC_POSITION
		.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
#endif // ! CALC_POSITION
		.build();

	std::vector<std::unique_ptr<ShBuffer>> cameraBuffers(ShSwapchain::MAX_FRAMES_IN_FLIGHT);
	for (int i = 0; i < cameraBuffers.size(); i++)
	{
		cameraBuffers[i] = std::make_unique<ShBuffer>(
			shDevice,
			sizeof(CameraExtentUBO),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		cameraBuffers[i]->map();
	}

	std::vector<VkDescriptorSet> lightDescriptorSets(ShSwapchain::MAX_FRAMES_IN_FLIGHT);
	auto& shadowBuffer = shadowRenderSystem.getBuffers();
	for (int i = 0; i < lightDescriptorSets.size(); i++)
	{
		auto cameraBufferInfo = cameraBuffers[i]->descriptorInfo();
		auto glboalBufferInfo = uboBuffers[i]->descriptorInfo();
		auto shadowBufferInfo = shadowBuffer[i]->descriptorInfo();
		ShDescriptorWriter(*lightingSetLayout0, *globalPool)
			.writeBuffer(0, &glboalBufferInfo)
			.writeBuffer(1, &cameraBufferInfo)
			.writeBuffer(2, &shadowBufferInfo)
			.build(lightDescriptorSets[i]);
	}

	std::vector<VkDescriptorSet> imageDescriptorSets(ShSwapchain::MAX_FRAMES_IN_FLIGHT);
	VkDescriptorImageInfo colorImageInfo = basePass.GetAlbedo();
	VkDescriptorImageInfo normalImageInfo = basePass.GetNormal();
	VkDescriptorImageInfo emissiveImageInfo = basePass.GetEmissive();
	VkDescriptorImageInfo depthImageInfo = basePass.GetDepth();
	VkDescriptorImageInfo shadowImageInfo = shadowPass.getShadowMapImageInfo();

	VkDescriptorImageInfo psotionImageInfo = basePass.GetPosition();
	for (int i = 0; i < imageDescriptorSets.size(); i++)
	{
		
		ShDescriptorWriter(*lightingSetLayout1, *globalPool)
			.writeImage(0, &colorImageInfo)
			.writeImage(1, &normalImageInfo)
			.writeImage(2, &emissiveImageInfo)
			.writeImage(3, &depthImageInfo)
			.writeImage(4, &shadowImageInfo)
#ifndef  CALC_POSITION
			.writeImage(5, &psotionImageInfo)
#endif // ! CALC_POSITION
			.build(imageDescriptorSets[i]);
	}

	std::vector<VkDescriptorSetLayout> lightSetLayouts { lightingSetLayout0->getDescriptorSetLayout(), lightingSetLayout1->getDescriptorSetLayout()};
	BlitRenderSystem lightingRenderSystem{ shDevice, lightPass.getRenderPass(), lightSetLayouts,  "shaders/spv/quad_vert.hlsl.spv", "shaders/spv/Lighting_frag.hlsl.spv" };

	PointLightSystem pointLightSystem{
		shDevice,
		shRenderer.getSwapChainRenderPass(),
		globalSetLayout->getDescriptorSetLayout() };

	auto blitSetLayout =
		ShDescriptorSetLayout::Builder(shDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

	std::vector<VkDescriptorSetLayout> blitSetLayouts{ blitSetLayout->getDescriptorSetLayout()};
	BlitRenderSystem blitRenderSystem{ shDevice, shRenderer.getSwapChainRenderPass(), blitSetLayouts, "shaders/spv/quad_vert.hlsl.spv", "shaders/spv/blit_frag.hlsl.spv" };

	std::vector<VkDescriptorSet> blitDescriptorSets(ShSwapchain::MAX_FRAMES_IN_FLIGHT);
	auto lightingColorImageInfo = lightPass.getColor();
	for (int i = 0; i < imageDescriptorSets.size(); i++)
	{
		ShDescriptorWriter(*blitSetLayout, *globalPool)
			.writeImage(0, &lightingColorImageInfo)
			.build(blitDescriptorSets[i]);
	}

	Camera2 camera{};
	camera.type = Camera2::CameraType::firstperson;
	camera.flipY = true;
	camera.movementSpeed = 2.0f;
	camera.setPosition(glm::vec3(0.443f, -0.163f, 2.433f));
	camera.setRotation(glm::vec3(-20.0f, 173.0f, 0.0f));
	camera.setPerspective(60.0f, (float)WIDTH / (float)HEIGHT, 0.1f, 256.0f);

	Input input(camera, *this);

	auto& pointLightGO = ShGameObject::getLight(gameObjects);
	auto currentTime = std::chrono::high_resolution_clock::now();
	while (!shWindow.shouldClose()) {
		glfwPollEvents();

		auto newTime = std::chrono::high_resolution_clock::now();
		auto frameTime =
			std::chrono::duration<double, std::milli>(newTime - currentTime).count();
		currentTime = newTime;
		auto sleeptime = 33.0f - frameTime;
		sleeptime = sleeptime > 0.0f ? sleeptime : 0.0f;
		Sleep((DWORD)sleeptime);

		float delta = (float)frameTime / 1000.0f;
		camera.update(delta);
		input.update(delta);
		for (auto& kv : gameObjects)
		{
			auto& obj = kv.second;
			if (obj.gltfmodel == nullptr)
				continue;

			obj.gltfmodel->updateAnimation(0, delta);
		}

		float aspect = shRenderer.getAspectRatio();

		if (auto commandBuffer = shRenderer.beginFrame())
		{
			int frameIndex = shRenderer.getFrameIndex();
			FrameInfo frameInfo{
				frameIndex,
				delta,
				lightUpdate,
				commandBuffer,
				camera,
				globalDescriptorSets[frameIndex],
				gameObjects };

			// update
			GlobalUbo ubo{};

			ubo.projection = camera.matrices.perspective;
			ubo.view = camera.matrices.view;
			ubo.viewPos = camera.viewPos;
			ubo.size = glm::vec4(WIDTH, HEIGHT, 1.0f / (float)WIDTH, 1.0f / (float) HEIGHT);
			ubo.camereInfo = glm::vec4(camera.getNearClip(), camera.getFarClip(), 1.0f / camera.getNearClip(), 1.0f / camera.getFarClip());

			CameraExtentUBO cameraubo{};

			glm::mat view = camera.matrices.view;
			view[3][0] = 0.0f;
			view[3][1] = 0.0f;
			view[3][2] = 0.0f;
			view[3][2] = 1.0f;

			glm::mat vp = camera.matrices.perspective * view;

			glm::mat inversevp = glm::inverse(vp);

			glm::vec4 lt = glm::vec4{ -1.0f, 1.0f, -1.0f, 1.0f };
			glm::vec4 rt = glm::vec4{ 1.0f, 1.0f, -1.0f, 1.0f };
			glm::vec4 lb = glm::vec4{ -1.0f, -1.0f, -1.0f, 1.0f };

			cameraubo.leftTop = inversevp * lt;
			glm::vec4 rtw = inversevp * rt;
			glm::vec4 lbw = inversevp * lb;
			cameraubo.left2Right = (rtw - cameraubo.leftTop);
			cameraubo.top2bottom = (lbw - cameraubo.leftTop);

			pointLightSystem.update(frameInfo, ubo);

			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			cameraBuffers[frameIndex]->writeToBuffer(&cameraubo);
			cameraBuffers[frameIndex]->flush();

			// shadow pass
			shadowRenderSystem.setupLight(pointLightGO, frameIndex);
			shadowPass.beginRenderPass(commandBuffer);
			shadowRenderSystem.renderGameObjects(frameInfo);
			shadowPass.endRenderPass(commandBuffer);

#ifdef DEFERRENDERING
			// base pass
			basePass.beginRenderPass(commandBuffer);
			baseRenderSystem.renderGameObjects(frameInfo);
			basePass.endRenderPass(commandBuffer);

			// lighting pass
			lightPass.beginRenderPass(commandBuffer);
			lightingRenderSystem.renderGameObjects(frameInfo, { lightDescriptorSets[frameIndex], imageDescriptorSets[frameIndex] });
			lightPass.endRenderPass(commandBuffer);
#endif

			// render
			shRenderer.beginSwapChainRenderPass(commandBuffer);

			// order here matters
			//simpleRenderSystem.renderGameObjects(frameInfo);
#ifdef DEFERRENDERING
			blitRenderSystem.renderGameObjects(frameInfo, { blitDescriptorSets[frameIndex] });
#else
			gltfRenderSystem.renderGameObjects(frameInfo);
#endif

			pointLightSystem.render(frameInfo);

			shRenderer.endSwapChainRenderPass(commandBuffer);
			shRenderer.endFrame();
		}
	}

	vkDeviceWaitIdle(shDevice.device());
}

const std::string MODEL_PATH = "models/buster_drone/busterDrone.gltf";
//const std::string MODEL_PATH = "models/sponza/sponza.gltf";
//const std::string MODEL_PATH = "models/cerberus/cerberus.gltf";

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
		pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(0.4f, 0.5f, 1.413f, 1.f));
		gameObjects.emplace(pointLight.getId(), std::move(pointLight));
	}
}
