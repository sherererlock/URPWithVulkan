#pragma once
#include "camera.h"
#include "ShDevice.h"
#include "sh_frame_info.h"
#include "ShGameObject.h"
#include "ShPipeline.h"

class RenderSystem
{
public:
	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.f };
		glm::mat4 normalMatrix{ 1.f };
	};

	RenderSystem(ShDevice& device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader);
	virtual ~RenderSystem();

	RenderSystem(const RenderSystem&) = delete;
	RenderSystem& operator=(const RenderSystem&) = delete;

	virtual void renderGameObjects(FrameInfo& frameInfo) = 0;
	virtual void createPipeline(VkRenderPass renderPass) = 0;
	virtual void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);

protected:

	ShDevice& lveDevice;

	std::unique_ptr<ShPipeline> lvePipeline;
	VkPipelineLayout pipelineLayout;
	std::string vertexShader;
	std::string fragmentShader;
};