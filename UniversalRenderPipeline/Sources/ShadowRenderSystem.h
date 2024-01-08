#pragma once
#include "RenderSystem.h"
#include "ShSwapchain.h"
class ShDescriptorSetLayout;
class ShDescriptorPool;

class ShadowRenderSystem : public RenderSystem
{
private:
	float lightFOV = 45.0f;
	float zNear = 1.0f;
	float zFar = 96.0f;
	std::unique_ptr<ShDescriptorSetLayout> setLayout;
	std::array<VkDescriptorSet, ShSwapchain::MAX_FRAMES_IN_FLIGHT> descriptorSets;

	std::array<std::unique_ptr<ShBuffer>, ShSwapchain::MAX_FRAMES_IN_FLIGHT> buffers;

public:
	ShadowRenderSystem(ShDevice& device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader);
	virtual ~ShadowRenderSystem();

	void setupDescriptorSet(ShDescriptorPool& pool);
	void setupLight(ShGameObject& light, int frameIndex);

	virtual void createPipeline(VkRenderPass renderPass) override;
	virtual void renderGameObjects(FrameInfo& frameInfo) override;
};

