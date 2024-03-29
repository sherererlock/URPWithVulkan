#include "ShadowRenderSystem.h"
#include "ShApp.h"
#include "macros.hlsl"
ShadowRenderSystem::ShadowRenderSystem(ShDevice& device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader, VkDescriptorImageInfo imageInfo)
	: RenderSystem(device, renderPass, vertexShader, fragmentShader), shadowMapInfo(imageInfo)
{
	setLayout = ShDescriptorSetLayout::Builder(lveDevice).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build();
	lightSetLayout = ShDescriptorSetLayout::Builder(lveDevice).
		addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT).
		addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT).build();

	std::vector<VkDescriptorSetLayout> setlayouts{ setLayout->getDescriptorSetLayout()};

#ifdef CPU_SKIN
	setlayouts.push_back(vkglTF::descriptorSetLayoutSkin);
#endif // CPU_SKIN

#ifdef CPU_ANIM
	setlayouts.push_back(vkglTF::descriptorSetLayoutAnim);
#endif

	createPipelineLayout(setlayouts);
	createPipeline(renderPass);

	for (int i = 0; i < ShSwapchain::MAX_FRAMES_IN_FLIGHT; i++)
	{
		buffers[i] = std::make_unique<ShBuffer>(device, sizeof(ShadowUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		buffers[i]->map();
	}
}

ShadowRenderSystem::~ShadowRenderSystem()
{
}

void ShadowRenderSystem::setupDescriptorSet(ShDescriptorPool& pool)
{
	for (int i = 0; i < descriptorSets.size(); i++) {
		auto bufferInfo = buffers[i]->descriptorInfo();
		ShDescriptorWriter(*setLayout, pool)
			.writeBuffer(0, &bufferInfo)
			.build(descriptorSets[i]);
	}

	for (int i = 0; i < lightSets.size(); i++)
	{
		auto bufferInfo = buffers[i]->descriptorInfo();

		ShDescriptorWriter(*lightSetLayout, pool)
			.writeBuffer(0, &bufferInfo)
			.writeImage(1, &shadowMapInfo)
			.build(lightSets[i]);
	}
}

void ShadowRenderSystem::setupLight(const ShGameObject& light, int frameIndex)
{
	glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
	glm::mat4 depthViewMatrix = glm::lookAt(light.transform.translation, glm::vec3(0.0f), glm::vec3(0, 1, 0));

	glm::mat4 vp = depthProjectionMatrix * depthViewMatrix;

	ShadowUBO ubo;

	ubo.lightVP = vp;
	ubo.shadowBias.x = (zFar - zNear) / ShAPP::ShadowResolution * 1.414213f;
	ubo.shadowBias.y = ShAPP::ShadowResolution;

	buffers[frameIndex]->writeToBuffer(&ubo);
	buffers[frameIndex]->flush();
}

void ShadowRenderSystem::createPipeline(VkRenderPass renderPass, uint32_t subpass)
{
	assert(pipelineLayout != nullptr && "GltfRenderSystem Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	ShPipeline::defaultPipelineConfigInfo(pipelineConfig, vkglTF::Vertex::getBindingDescriptions, vkglTF::Vertex::getAttributeDescription);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	lvePipeline = std::make_unique<ShPipeline>(
		lveDevice,
		vertexShader,
		fragmentShader,
		pipelineConfig);
}

void ShadowRenderSystem::renderGameObjects(FrameInfo& frameInfo, VkCommandBuffer commandBuffer)
{
	lvePipeline->bind(commandBuffer);

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0,
		1,
		&descriptorSets[frameInfo.frameIndex],
		0,
		nullptr);

	for (auto& kv : frameInfo.gameObjects)
	{
		auto& obj = kv.second;
		if (obj.gltfmodel == nullptr)
			continue;

		obj.transform.translation = glm::vec3(glm::vec4(obj.transform.translation, 1.f));
		obj.gltfmodel->modelMatrix = obj.transform.mat4();

#ifdef CPU_SKIN
		obj.gltfmodel->draw(commandBuffer, RenderFlags::BindSkin, pipelineLayout, 0, frameInfo.frameIndex);
#elif defined CPU_ANIM
		obj.gltfmodel->draw(commandBuffer, RenderFlags::BindAnim, pipelineLayout, 0, frameInfo.frameIndex);
#else
		obj.gltfmodel->draw(commandBuffer, 0, pipelineLayout, 0, frameInfo.frameIndex);
#endif
	}
}
