#pragma once


#include "camera.h"
#include "ShDevice.h"
#include "sh_frame_info.h"
#include "ShGameObject.h"
#include "ShPipeline.h"
#include "RenderSystem.h"

class SimpleRenderSystem : public RenderSystem
{
public:
	SimpleRenderSystem(
		ShDevice& device, VkRenderPass renderPass, const std::vector<VkDescriptorSetLayout>& setLayouts, std::string vertexShader, std::string fragmentShader);
	~SimpleRenderSystem();

	virtual void renderGameObjects(FrameInfo& frameInfo) override;
};

