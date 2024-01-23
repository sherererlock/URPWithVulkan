#include "SimpleRenderSystem.h"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

SimpleRenderSystem::SimpleRenderSystem(
	ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayout, std::string vertexShader, std::string fragmentShader)
	: RenderSystem(device, renderPass, vertexShader, fragmentShader) 
{
	createPipelineLayout(setlayout);
	createPipeline(renderPass);
}

SimpleRenderSystem::~SimpleRenderSystem() 
{
}

void SimpleRenderSystem::createPipeline(VkRenderPass renderPass, uint32_t subpass) {
	assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	ShPipeline::defaultPipelineConfigInfo(pipelineConfig, ShModel::Vertex::getBindingDescriptions, ShModel::Vertex::getAttributeDescriptions);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	lvePipeline = std::make_unique<ShPipeline>(
		lveDevice,
		vertexShader,
		fragmentShader,
		pipelineConfig);
}

void SimpleRenderSystem::renderGameObjects(FrameInfo& frameInfo, VkCommandBuffer commandBuffer)
{
	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	lvePipeline->bind(commandBuffer);

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0,
		1,
		&frameInfo.globalDescriptorSet,
		0,
		nullptr);

	for (auto& kv : frameInfo.gameObjects) {
		auto& obj = kv.second;
		if (obj.model == nullptr)
			continue;

		SimplePushConstantData push{};
		push.modelMatrix = obj.transform.mat4();
		push.normalMatrix = obj.transform.normalMatrix();

		vkCmdPushConstants(
			commandBuffer,
			pipelineLayout,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0,
			sizeof(SimplePushConstantData),
			&push);

		obj.model->bind(commandBuffer);
		obj.model->draw(commandBuffer);
	}
}