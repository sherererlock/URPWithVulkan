#pragma once
#include "RenderSystem.h"

class ShadowRenderSystem;

class GltfRenderSystem : public RenderSystem
{
private:
	ShadowRenderSystem* shadowRenderSystem;
	uint32_t colorBlendAttachmentCount;

public:
	GltfRenderSystem(ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayouts, std::string vertexShader, std::string fragmentShader, ShadowRenderSystem* rendersystem = nullptr, uint32_t colorBlendAttachmentCount = 1);
	virtual ~GltfRenderSystem() {}

	virtual void renderGameObjects(FrameInfo& frameInfo) override;
	virtual void createPipeline(VkRenderPass renderPass) override;
	virtual void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts) override;
};
