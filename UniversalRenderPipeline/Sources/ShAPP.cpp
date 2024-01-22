
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


#include "InputController.h"


#include "macros.hlsl"

#define MAX_SET_NUM 30
#define MAX_UNIFORM_BUFFER_NUM 30
#define MAX_SAMPLER_BUFFER_NUM 30

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

	blitDescriptorSets.resize(ShSwapchain::MAX_FRAMES_IN_FLIGHT);
	lightDescriptorSets.resize(ShSwapchain::MAX_FRAMES_IN_FLIGHT);
	imageDescriptorSets.resize(ShSwapchain::MAX_FRAMES_IN_FLIGHT);
}

ShAPP::~ShAPP()
{
	UIOverlay.freeResources();
}

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

#ifdef SHADOW
	shadowPass = std::make_unique<ShadowPass>(shDevice,ShadowResolution, ShadowResolution);
	shadowRenderSystem = std::make_unique <ShadowRenderSystem>( shDevice, shadowPass->getRenderPass(), "shaders/spv/shadow_vert.hlsl.spv", "shaders/spv/shadow_frag.hlsl.spv", shadowPass->getShadowMapImageInfo() );
	shadowRenderSystem->setupDescriptorSet(*globalPool);
#endif

	basePass = std::make_unique<BasePass>( shDevice, WIDTH, HEIGHT );
	lightPass = std::make_unique<LightingPass>(shDevice, WIDTH, HEIGHT, shRenderer.getFormat());

	std::vector<VkDescriptorSetLayout> setlayouts{ globalSetLayout->getDescriptorSetLayout() };

	//SimpleRenderSystem simpleRenderSystem{
	//	shDevice,
	//	shRenderer.getSwapChainRenderPass(),
	//	"shaders/spv/simple_shader.vert.spv", "shaders/spv/simple_shader.frag.spv", };

	//simpleRenderSystem.createPipelineLayout(setlayouts);
	//simpleRenderSystem.createPipeline(shRenderer.getSwapChainRenderPass());

	std::string vs_shader = "shaders/spv/gbuffer_vert.hlsl.spv";
	std::string ps_shader = "shaders/spv/gbuffer_frag.hlsl.spv";
	uint32_t attachmentCount = 3;
	ShadowRenderSystem* shadowRenderSystemPtr = nullptr;

#ifndef CALC_POSITION
	attachmentCount = 4;
#endif

#ifndef DEFERRENDERING
	vs_shader = "shaders/spv/pbr_vert.hlsl.spv";
	ps_shader = "shaders/spv/pbr_frag.hlsl.spv";
	shadowRenderSystemPtr = &shadowRenderSystem;
	attachmentCount = 1;
#endif

#ifdef CPU_SKIN
	vs_shader = "shaders/spv/gbuffer_skin.vert.spv";
#elif defined CPU_ANIM
	vs_shader = "shaders/spv/gbuffer_anim.vert.spv";
#endif

	baseRenderSystem = std::make_unique<GltfRenderSystem>(shDevice, basePass->getRenderPass(), setlayouts, vs_shader, ps_shader, shadowRenderSystemPtr, attachmentCount);

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

#ifdef SHADOW
	auto& shadowBuffer = shadowRenderSystem->getBuffers();
#endif

	for (int i = 0; i < lightDescriptorSets.size(); i++)
	{
		auto cameraBufferInfo = cameraBuffers[i]->descriptorInfo();
		auto glboalBufferInfo = uboBuffers[i]->descriptorInfo();
#ifdef SHADOW
		auto shadowBufferInfo = shadowBuffer[i]->descriptorInfo();
#endif
		ShDescriptorWriter(*lightingSetLayout0, *globalPool)
			.writeBuffer(0, &glboalBufferInfo)
			.writeBuffer(1, &cameraBufferInfo)
#ifdef SHADOW
			.writeBuffer(2, &shadowBufferInfo)
#endif
			.build(lightDescriptorSets[i]);
	}

	VkDescriptorImageInfo colorImageInfo = basePass->GetAlbedo();
	VkDescriptorImageInfo normalImageInfo = basePass->GetNormal();
	VkDescriptorImageInfo emissiveImageInfo = basePass->GetEmissive();
	VkDescriptorImageInfo depthImageInfo = basePass->GetDepth();

	uint32_t positionindex = 4;
#ifdef SHADOW
	positionindex = 5;
	VkDescriptorImageInfo shadowImageInfo = shadowPass->getShadowMapImageInfo();
#endif

	VkDescriptorImageInfo psotionImageInfo = basePass->GetPosition();
	for (int i = 0; i < imageDescriptorSets.size(); i++)
	{
		
		ShDescriptorWriter(*lightingSetLayout1, *globalPool)
			.writeImage(0, &colorImageInfo)
			.writeImage(1, &normalImageInfo)
			.writeImage(2, &emissiveImageInfo)
			.writeImage(3, &depthImageInfo)
#ifdef SHADOW
			.writeImage(4, &shadowImageInfo)
#endif

#ifndef  CALC_POSITION
			.writeImage(positionindex, &psotionImageInfo)
#endif // ! CALC_POSITION
			.build(imageDescriptorSets[i]);
	}

	std::vector<VkDescriptorSetLayout> lightSetLayouts { lightingSetLayout0->getDescriptorSetLayout(), lightingSetLayout1->getDescriptorSetLayout()};
	lightingRenderSystem = std::make_unique<BlitRenderSystem>(shDevice, lightPass->getRenderPass(), lightSetLayouts,  "shaders/spv/quad_vert.hlsl.spv", "shaders/spv/Lighting_frag.hlsl.spv");

	pointLightSystem = std::make_unique<PointLightSystem>(
		shDevice,
		shRenderer.getSwapChainRenderPass(),
		globalSetLayout->getDescriptorSetLayout());

	auto blitSetLayout =
		ShDescriptorSetLayout::Builder(shDevice)
		.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build();

	std::vector<VkDescriptorSetLayout> blitSetLayouts{ blitSetLayout->getDescriptorSetLayout()};
	blitRenderSystem = std::make_unique<BlitRenderSystem>(shDevice, shRenderer.getSwapChainRenderPass(), blitSetLayouts, "shaders/spv/quad_vert.hlsl.spv", "shaders/spv/blit_frag.hlsl.spv");

	auto lightingColorImageInfo = lightPass->getColor();
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

	createUIOverlay();
	updateOverlay(input);
	uint32_t frameCounter = 0;

	std::chrono::time_point<std::chrono::high_resolution_clock> lastTimestamp, tPrevEnd;

	while (!shWindow.shouldClose()) {
		glfwPollEvents();

		auto tStart = std::chrono::high_resolution_clock::now();

		auto sleeptime = 33.0f - frameTimer * 1000.0f;
		sleeptime = sleeptime > 0.0f ? sleeptime : 0.0f;
		//Sleep((DWORD)sleeptime);
		camera.update(frameTimer);
		input.update(frameTimer);

		for (auto& kv : gameObjects)
		{
			auto& obj = kv.second;
			if (obj.gltfmodel == nullptr)
				continue;

			obj.gltfmodel->updateAnimation(0, frameTimer);
		}

		float aspect = shRenderer.getAspectRatio();

		// update
		GlobalUbo ubo{};

		ubo.projection = camera.matrices.perspective;
		ubo.view = camera.matrices.view;
		ubo.viewPos = camera.viewPos;
		ubo.size = glm::vec4(WIDTH, HEIGHT, 1.0f / (float)WIDTH, 1.0f / (float)HEIGHT);
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

		shRenderer.beginFrame();
		int frameIndex = shRenderer.getFrameIndex();
		FrameInfo frameInfo{
			frameIndex,
			frameTimer,
			lightUpdate,
			camera,
			globalDescriptorSets[frameIndex],
			gameObjects };

		pointLightSystem->update(frameInfo, ubo);

		if (camera.updated)
		{
			uboBuffers[frameIndex]->writeToBuffer(&ubo);
			uboBuffers[frameIndex]->flush();

			cameraBuffers[frameIndex]->writeToBuffer(&cameraubo);
			cameraBuffers[frameIndex]->flush();
		}

		updateOverlay(input);
		if (bufferCount < ShSwapchain::MAX_FRAMES_IN_FLIGHT)
		{
			buildCommandBuffer(frameIndex, frameInfo);
			bufferCount++;
		}

		shRenderer.endFrame();
		frameCounter++;
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		frameTimer = (float)tDiff / 1000.0f;
		float fpsTimer = (float)(std::chrono::duration<double, std::milli>(tEnd - lastTimestamp).count());
		if (fpsTimer > 1000.0f)
		{
			lastFPS = static_cast<uint32_t>((float)frameCounter * (1000.0f / fpsTimer));
			frameCounter = 0;
			lastTimestamp = tEnd;
		}
		//std::cout << lastFPS << std::endl;
	}

	vkDeviceWaitIdle(shDevice.device());
}

void ShAPP::nextFrame()
{
}

void ShAPP::updateOverlay(Input& input)
{
	ImGuiIO& io = ImGui::GetIO();

	io.DisplaySize = ImVec2((float)WIDTH, (float)HEIGHT);
	io.DeltaTime = frameTimer;

	io.MousePos = ImVec2(input.mousePos.x, input.mousePos.y);
	io.MouseDown[0] = input.mouseButtons.left && UIOverlay.visible;
	io.MouseDown[1] = input.mouseButtons.right && UIOverlay.visible;
	io.MouseDown[2] = input.mouseButtons.middle && UIOverlay.visible;

	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
	ImGui::SetNextWindowPos(ImVec2(10 * UIOverlay.scale, 10 * UIOverlay.scale));
	ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
	ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
	ImGui::TextUnformatted("Vulkan URP");
	ImGui::TextUnformatted("RTX 4060TI");
	ImGui::Text("%.2f ms/frame (%.1d fps)", (1000.0f / lastFPS), lastFPS);

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 5.0f * UIOverlay.scale));
#endif
	ImGui::PushItemWidth(110.0f * UIOverlay.scale);
	OnUpdateUIOverlay();
	ImGui::PopItemWidth();
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	ImGui::PopStyleVar();
#endif

	ImGui::End();
	ImGui::PopStyleVar();
	ImGui::Render();

	if (UIOverlay.update() || UIOverlay.updated) {
		bufferCount = 0;
		UIOverlay.updated = false;
	}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	if (mouseButtons.left) {
		mouseButtons.left = false;
	}
#endif
}

void ShAPP::drawUI(const VkCommandBuffer commandBuffer)
{
	if (UIOverlay.visible) {
		const VkViewport viewport = Initializers::viewport((float)WIDTH, (float)HEIGHT, 0.0f, 1.0f);
		const VkRect2D scissor = Initializers::rect2D(WIDTH, HEIGHT, 0, 0);
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

		UIOverlay.draw(commandBuffer);
	}
}

void ShAPP::createUIOverlay()
{
	VkFormat dformat = shDevice.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

	UIOverlay.device = &shDevice;
	UIOverlay.queue = shDevice.graphicsQueue();
	std::vector<char> vs_code = ShPipeline::readFile("shaders/spv/uioverlay.vert.spv");
	std::vector<char> ps_code = ShPipeline::readFile("shaders/spv/uioverlay.frag.spv");
	VkShaderModule vs_shaderModule, ps_shaderModule;
	ShPipeline::createShaderModule(shDevice, vs_code, &vs_shaderModule);
	ShPipeline::createShaderModule(shDevice, ps_code, &ps_shaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = vs_shaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;
	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = ps_shaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	UIOverlay.shaders = { shaderStages[0], shaderStages[1] };

	UIOverlay.prepareResources();
	UIOverlay.preparePipeline(nullptr, shRenderer.getSwapChainRenderPass(), shRenderer.getFormat(), dformat);

	vkDestroyShaderModule(shDevice.device(), vs_shaderModule, nullptr);
	vkDestroyShaderModule(shDevice.device(), ps_shaderModule, nullptr);
}

void ShAPP::OnUpdateUIOverlay()
{

}

void ShAPP::buildCommandBuffer(uint32_t frameIndex, FrameInfo& frameInfo)
{
	auto commandBuffer = shRenderer.beginCommandBuffer();
	auto& pointLightGO = ShGameObject::getLight(gameObjects);

#ifdef  SHADOW
	// shadow pass
	shadowRenderSystem->setupLight(pointLightGO, frameIndex);
	shadowPass->beginRenderPass(commandBuffer);
	shadowRenderSystem->renderGameObjects(frameInfo, commandBuffer);
	shadowPass->endRenderPass(commandBuffer);
#endif //  SHADOW

#ifdef DEFERRENDERING
	// base pass
	basePass->beginRenderPass(commandBuffer);
	baseRenderSystem->renderGameObjects(frameInfo, commandBuffer);
	basePass->endRenderPass(commandBuffer);

	// lighting pass
	lightPass->beginRenderPass(commandBuffer);
	lightingRenderSystem->renderGameObjects(frameInfo, { lightDescriptorSets[frameIndex], imageDescriptorSets[frameIndex] }, commandBuffer);
	lightPass->endRenderPass(commandBuffer);
#endif

	// render
	shRenderer.beginSwapChainRenderPass(commandBuffer);

	// order here matters
	//simpleRenderSystem.renderGameObjects(frameInfo);
#ifdef DEFERRENDERING
	blitRenderSystem->renderGameObjects(frameInfo, { blitDescriptorSets[frameIndex] }, commandBuffer);
#else
	gltfRenderSystem.renderGameObjects(frameInfo);
#endif

	pointLightSystem->render(frameInfo, commandBuffer);

	drawUI(commandBuffer);
	shRenderer.endSwapChainRenderPass(commandBuffer);

	shRenderer.endCommandBuffer();
}

const std::string MODEL_PATH = "Assets/models/buster_drone/busterDrone.gltf";
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
	//const uint32_t glTFLoadingFlags = vkglTF::FileLoadingFlags::PreTransformVertices;
	const uint32_t glTFLoadingFlags = 0;
	std::shared_ptr<Model> gltfModel = std::make_shared<Model>();
	gltfModel->loadFromFile(MODEL_PATH, &shDevice, shDevice.graphicsQueue(), glTFLoadingFlags);
	gltfModel->updateAnimation(0, 0.0f);

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
