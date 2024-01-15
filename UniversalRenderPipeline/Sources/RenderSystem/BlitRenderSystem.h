#pragma once
#include "RenderSystem.h"
class BlitRenderSystem : public RenderSystem
{
public:
	BlitRenderSystem(ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayouts, std::string vertexShader, std::string fragmentShader);
	virtual ~BlitRenderSystem() {}

	void renderGameObjects(FrameInfo& frameInfo, const std::vector<VkDescriptorSet>& descriptorSets);

	virtual void renderGameObjects(FrameInfo& frameInfo) override {};
	virtual void createPipeline(VkRenderPass renderPass) override;
	virtual void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts) override;
};
