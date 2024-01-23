#pragma once
#include "RenderSystem.h"
#include "ShDescriptors.h"
class ShadowRenderSystem;

class GltfRenderSystem : public RenderSystem
{
private:
	ShadowRenderSystem* shadowRenderSystem;
	uint32_t colorBlendAttachmentCount;

public:
	GltfRenderSystem(ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayouts, std::string vertexShader, std::string fragmentShader, ShadowRenderSystem* rendersystem = nullptr, uint32_t colorBlendAttachmentCount = 1, uint32_t subpass = 0);
	virtual ~GltfRenderSystem() {}

	virtual void renderGameObjects(FrameInfo& frameInfo, VkCommandBuffer commandBuffer) override;
	virtual void createPipeline(VkRenderPass renderPass, uint32_t subpass = 0) override;
	virtual void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts) override;
};

