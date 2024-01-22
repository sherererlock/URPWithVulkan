#pragma once

#include "RenderSystem.h"

class SimpleRenderSystem : public RenderSystem
{
public:
	SimpleRenderSystem(
		ShDevice& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setlayout, std::string vertexShader, std::string fragmentShader);
	virtual ~SimpleRenderSystem();

	virtual void renderGameObjects(FrameInfo& frameInfo, VkCommandBuffer commandBuffer) override;

	virtual void createPipeline(VkRenderPass renderPass) override;
};

