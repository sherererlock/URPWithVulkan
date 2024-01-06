#pragma once
#include "RenderSystem.h"

class GltfRenderSystem : public RenderSystem
{
public:

	GltfRenderSystem(
		ShDevice& device, VkRenderPass renderPass, const std::vector<VkDescriptorSetLayout>& setLayouts, std::string vertexShader, std::string fragmentShader);
	virtual ~GltfRenderSystem() {}

	virtual void renderGameObjects(FrameInfo& frameInfo) override;
	virtual void createPipeline(VkRenderPass renderPass) override;
};

