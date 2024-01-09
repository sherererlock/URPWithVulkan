#include "ShadowRenderSystem.h"

ShadowRenderSystem::ShadowRenderSystem(ShDevice& device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader, VkDescriptorImageInfo imageInfo)
	: RenderSystem(device, renderPass, vertexShader, fragmentShader), shadowMapInfo(imageInfo)
{
	setLayout = ShDescriptorSetLayout::Builder(lveDevice).addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT).build();
	lightSetLayout = ShDescriptorSetLayout::Builder(lveDevice).
		addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT).
		addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT|VK_SHADER_STAGE_FRAGMENT_BIT).build();

	std::vector<VkDescriptorSetLayout> setlayouts{ setLayout->getDescriptorSetLayout()};
	createPipelineLayout(setlayouts);
	createPipeline(renderPass);

	for (int i = 0; i < ShSwapchain::MAX_FRAMES_IN_FLIGHT; i++)
	{
		buffers[i] = std::make_unique<ShBuffer>(device, sizeof(glm::mat4), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
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
	buffers[frameIndex]->writeToBuffer(&vp);
	buffers[frameIndex]->flush();
}

void ShadowRenderSystem::createPipeline(VkRenderPass renderPass)
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

void ShadowRenderSystem::renderGameObjects(FrameInfo& frameInfo)
{
	lvePipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(
		frameInfo.commandBuffer,
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

		SimplePushConstantData push{};
		auto rotateLight = glm::rotate(glm::mat4(1.f), 0.5f * frameInfo.frameTime, { 0.f, -1.f, 0.f });
		obj.transform.translation = glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.f));
		push.modelMatrix = obj.transform.mat4();
		push.normalMatrix = obj.transform.normalMatrix();

		vkCmdPushConstants(
			frameInfo.commandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(SimplePushConstantData),
			&push);

		obj.gltfmodel->draw(frameInfo.commandBuffer);
	}
}
