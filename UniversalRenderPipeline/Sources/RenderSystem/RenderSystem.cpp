#include <stdexcept>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/constants.hpp>

#include "RenderSystem.h"

RenderSystem::RenderSystem(
	ShDevice& device, VkRenderPass renderPass, std::string vs, std::string fs)
	: lveDevice{ device }, pipelineLayout{VK_NULL_HANDLE},vertexShader(vs), fragmentShader(fs)
{
}

RenderSystem::~RenderSystem() {
	vkDestroyPipelineLayout(lveDevice.device(), pipelineLayout, nullptr);
}

void RenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setlayouts) {
	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};
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
