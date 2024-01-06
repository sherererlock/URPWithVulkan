#pragma once
#include "camera.h"
#include "ShDevice.h"
#include "sh_frame_info.h"
#include "ShGameObject.h"
#include "ShPipeline.h"

class RenderSystem
{
public:
	RenderSystem(
		ShDevice& device, VkRenderPass renderPass, const std::vector<VkDescriptorSetLayout>& setLayouts, std::string vertexShader, std::string fragmentShader);
	~RenderSystem();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	virtual void renderGameObjects(FrameInfo& frameInfo);

protected:
	void createPipelineLayout(const std::vector<VkDescriptorSetLayout>& setLayouts);
	void createPipeline(VkRenderPass renderPass);

	ShDevice& lveDevice;

	std::unique_ptr<ShPipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
	std::string vertexShader;
	std::string fragmentShader;
};