#pragma once
#include "camera.h"
#include "ShDevice.h"
#include "sh_frame_info.h"
#include "ShGameObject.h"
#include "ShPipeline.h"

class RenderSystem
{
public:

	RenderSystem(ShDevice& device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader);
	virtual ~RenderSystem();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	virtual void renderGameObjects(FrameInfo& frameInfo,
		VkCommandBuffer commandBuffer) = 0;
	virtual void createPipeline(VkRenderPass renderPass) = 0;
	virtual void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);

protected:

	ShDevice& lveDevice;

	std::unique_ptr<ShPipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
	std::string vertexShader;
	std::string fragmentShader;
};