#include "BlitRenderSystem.h"

BlitRenderSystem::BlitRenderSystem(ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayouts, std::string vertexShader, std::string fragmentShader)
	: RenderSystem(device, renderPass, vertexShader, fragmentShader)
{
	createPipelineLayout(setlayouts);
	createPipeline(renderPass);
}

void BlitRenderSystem::renderGameObjects(FrameInfo& frameInfo)
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
}

std::vector<VkVertexInputBindingDescription> getEmptyInputBinding()
{
	return {};
}

std::vector<VkVertexInputAttributeDescription> getEmptyAttributeDescriptions()
{
	return {};
}

void BlitRenderSystem::createPipeline(VkRenderPass renderPass)
{
	assert(pipelineLayout != nullptr && "GltfRenderSystem Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	ShPipeline::defaultPipelineConfigInfo(pipelineConfig, getEmptyInputBinding, getEmptyAttributeDescriptions);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	lvePipeline = std::make_unique<ShPipeline>(
		lveDevice,
		vertexShader,
		fragmentShader,
		pipelineConfig);
}

void BlitRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
{
	RenderSystem::createPipelineLayout(setLayouts);
}
