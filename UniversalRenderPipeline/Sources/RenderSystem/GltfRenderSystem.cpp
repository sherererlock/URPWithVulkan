#include "GltfRenderSystem.h"
#include "VulkanglTFModel.h"
#include "ShadowRenderSystem.h"
#include "macros.hlsl"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/constants.hpp>

#include <array>
#include <cassert>
#include <stdexcept>

GltfRenderSystem::GltfRenderSystem(
	ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayouts, std::string vertexShader, std::string fragmentShader, ShadowRenderSystem* rendersystem, uint32_t colorBlendAttachmentCount, uint32_t subpass)
	: RenderSystem(device, renderPass, vertexShader, fragmentShader), shadowRenderSystem(rendersystem), colorBlendAttachmentCount(colorBlendAttachmentCount)
{
	createPipelineLayout(setlayouts);
	createPipeline(renderPass, subpass);
}

void GltfRenderSystem::createPipeline(VkRenderPass renderPass, uint32_t subpass)
{
	assert(pipelineLayout != nullptr && "GltfRenderSystem Cannot create pipeline before pipeline layout");

	PipelineConfigInfo pipelineConfig{};
	ShPipeline::defaultPipelineConfigInfo(pipelineConfig, vkglTF::Vertex::getBindingDescriptions, vkglTF::Vertex::getAttributeDescriptions, colorBlendAttachmentCount);
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pipelineLayout;
	pipelineConfig.subpass = subpass;
	lvePipeline = std::make_unique<ShPipeline>(
		lveDevice,
		vertexShader,
		fragmentShader,
		pipelineConfig);
}

void GltfRenderSystem::createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts)
{
	std::vector<VkDescriptorSetLayout> layouts = setLayouts;
	if (vkglTF::descriptorSetLayoutImage != VK_NULL_HANDLE)
	{
		layouts.push_back(vkglTF::descriptorSetLayoutImage);
	}
	if (shadowRenderSystem != nullptr)
	{
		layouts.push_back(shadowRenderSystem->getLightSetLayout());
	}

#ifdef CPU_SKIN
	layouts.push_back(vkglTF::descriptorSetLayoutSkin);
#endif // CPU_SKIN

#ifdef CPU_ANIM
	layouts.push_back(vkglTF::descriptorSetLayoutAnim);
#endif

	RenderSystem::createPipelineLayout(layouts);
}

void GltfRenderSystem::SetupSet(const std::array<VkDescriptorSet, ShSwapchain::MAX_FRAMES_IN_FLIGHT>& sets)
{
	descriptorSets = sets;
}

void GltfRenderSystem::renderGameObjects(FrameInfo& frameInfo, VkCommandBuffer commandBuffer)
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

	if (shadowRenderSystem != nullptr)
	{
		VkDescriptorSet set = shadowRenderSystem->getLightSet(frameInfo.frameIndex);
		vkCmdBindDescriptorSets(
			commandBuffer,
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

		obj.transform.translation = glm::vec3(glm::vec4(obj.transform.translation, 1.f));
		obj.gltfmodel->modelMatrix = obj.transform.mat4();


		RenderFlags flag = bindimage ? RenderFlags::BindImages : (RenderFlags)0;
#ifdef CPU_SKIN
	obj.gltfmodel->draw(commandBuffer, flag | RenderFlags::BindSkin, pipelineLayout, 1, frameInfo.frameIndex);
#elif defined CPU_ANIM
	obj.gltfmodel->draw(commandBuffer, flag | RenderFlags::BindAnim, pipelineLayout, 1, frameInfo.frameIndex);
#else
	obj.gltfmodel->draw(commandBuffer, flag, pipelineLayout, 1, frameInfo.frameIndex);
#endif
	}
}
