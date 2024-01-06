#include <stdexcept>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/constants.hpp>

#include "RenderSystem.h"

RenderSystem::RenderSystem(
	ShDevice& device, VkRenderPass renderPass, const std::vector<VkDescriptorSetLayout>& setlayouts, std::string vs, std::string fs)
	: lveDevice{ device } , vertexShader(vs), fragmentShader(fs)
{
	createPipelineLayout(setlayouts);

}

RenderSystem::~RenderSystem() {
	vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void RenderSystem::createPipelineLayout(const std::vector<VkDescriptorSetLayout>& setlayouts) {
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setlayouts.size());
	pipelineLayoutInfo.pSetLayouts = setlayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
	if (vkCreatePipelineLayout(lveDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) !=
		VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
}
