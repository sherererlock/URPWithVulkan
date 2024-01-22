#pragma once
// std
#include <memory>
#include <vector>

#include "camera.h"
#include "shdevice.h"
#include "sh_frame_info.h"
#include "shgameobject.h"
#include "shpipeline.h"

class PointLightSystem {
public:
	PointLightSystem(
		ShDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~PointLightSystem();

	PointLightSystem(const PointLightSystem&) = delete;
	PointLightSystem& operator=(const PointLightSystem&) = delete;

	void update(FrameInfo& frameInfo, GlobalUbo& ubo);
	void render(FrameInfo& frameInfo, VkCommandBuffer commandBuffer);

private:
	void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void createPipeline(VkRenderPass renderPass);

	ShDevice& lveDevice;

	std::unique_ptr<ShPipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
};

