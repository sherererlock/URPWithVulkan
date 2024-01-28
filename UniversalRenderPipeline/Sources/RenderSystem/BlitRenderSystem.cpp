#include "BlitRenderSystem.h"

BlitRenderSystem::BlitRenderSystem(ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayouts, std::string vertexShader, std::string fragmentShader, uint32_t subpass)
	: RenderSystem(device, renderPass, vertexShader, fragmentShader)
{
	createPipelineLayout(setlayouts);
	createPipeline(renderPass, subpass);
}

void BlitRenderSystem::renderGameObjects(FrameInfo& frameInfo, const std::vector<VkDescriptorSet>& descriptorSets, VkCommandBuffer commandBuffer)
{
	lvePipeline->bind(commandBuffer);

	for (int i = 0; i < descriptorSets.size(); i++)
	{
		vkCmdBindDescriptorSets(
			commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			i,
			1,
			&descriptorSets[i],
			0,
			nullptr);
	}

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}

static std::vector<VkVertexInputBindingDescription> getEmptyInputBinding()
{
	return {};
}

static std::vector<VkVertexInputAttributeDescription> getEmptyAttributeDescriptions()
{
	return {};
}

void BlitRenderSystem::createPipeline(VkRenderPass renderPass, uint32_t subpass)
{
	assert(pipelineLayout != nullptr && "GltfRenderSystem Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	ShPipeline::defaultPipelineConfigInfo(pipelineConfig, getEmptyInputBinding, getEmptyAttributeDescriptions);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipelineConfig.subpass = subpass;
	//pipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
	pipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
	pipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	pipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	lvePipeline = std::make_unique<ShPipeline>(
		lveDevice,
		vertexShader,
		fragmentShader,
		pipelineConfig);
}

void BlitRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
	pipelineLayoutInfo.pSetLayouts = setLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
