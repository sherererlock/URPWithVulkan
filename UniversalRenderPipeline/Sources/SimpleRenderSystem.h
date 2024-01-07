#pragma once

#include "RenderSystem.h"

class SimpleRenderSystem : public RenderSystem
{
public:
	SimpleRenderSystem(
		ShDevice& device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader);
	virtual ~SimpleRenderSystem();

	virtual void renderGameObjects(FrameInfo& frameInfo) override;

	virtual void createPipeline(VkRenderPass renderPass) override;
};

