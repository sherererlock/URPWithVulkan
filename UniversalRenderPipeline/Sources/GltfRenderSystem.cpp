#include "GltfRenderSystem.h"
#include "VulkanglTFModel.h"
#include "ShadowRenderSystem.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

GltfRenderSystem::GltfRenderSystem(
	ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayouts, std::string vertexShader, std::string fragmentShader, ShadowRenderSystem* rendersystem)
	: RenderSystem(device, renderPass, vertexShader, fragmentShader), shadowRenderSystem(rendersystem)
{
	createPipelineLayout(setlayouts);
	createPipeline(renderPass);
}

void GltfRenderSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "GltfRenderSystem Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	ShPipeline::defaultPipelineConfigInfo(pipelineConfig, vkglTF::Vertex::getBindingDescriptions, vkglTF::Vertex::getAttributeDescriptions);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	lvePipeline = std::make_unique<ShPipeline>(
		lveDevice,
		vertexShader,
		fragmentShader,
		pipelineConfig);
}

void GltfRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
{
	if (vkglTF::descriptorSetLayoutImage != VK_NULL_HANDLE)
	{
		setLayouts.push_back(vkglTF::descriptorSetLayoutImage);
	}
	if (shadowRenderSystem != nullptr)
	{
		setLayouts.push_back(shadowRenderSystem->getLightSetLayout());
	}
	RenderSystem::createPipelineLayout(setLayouts);
}

void GltfRenderSystem::renderGameObjects(FrameInfo& frameInfo)
{
	lvePipeline->bind(frameInfo.commandBuffer);

	vkCmdBindDescriptorSets(
		frameInfo.commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0,
		1,
		&frameInfo.globalDescriptorSet,
		0,
		nullptr);

	if (shadowRenderSystem != nullptr)
	{
		VkDescriptorSet set = shadowRenderSystem->getLightSet(frameInfo.frameIndex);
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			2,
			1,
			&set,
			0,
			nullptr);
	}

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

		obj.gltfmodel->draw(frameInfo.commandBuffer, RenderFlags::BindImages, pipelineLayout, 1);
	}
}
