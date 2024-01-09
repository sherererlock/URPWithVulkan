#pragma once
#include "RenderSystem.h"
#include "ShSwapchain.h"
#include "ShDescriptors.h"

class ShadowRenderSystem : public RenderSystem
{
private:
	struct ShadowUBO
	{
		glm::mat4 lightVP;
		glm::vec4 shadowBias;
	};

private:
	float lightFOV = 60.0f;
	float zNear = 0.1f;
	float zFar = 96.0f;
	std::unique_ptr<ShDescriptorSetLayout> setLayout;
	std::array<VkDescriptorSet, ShSwapchain::MAX_FRAMES_IN_FLIGHT> descriptorSets;
	std::array<std::unique_ptr<ShBuffer>, ShSwapchain::MAX_FRAMES_IN_FLIGHT> buffers;

	std::unique_ptr<ShDescriptorSetLayout> lightSetLayout;
	std::array<VkDescriptorSet, ShSwapchain::MAX_FRAMES_IN_FLIGHT> lightSets;
	VkDescriptorImageInfo shadowMapInfo;


public:
	ShadowRenderSystem(ShDevice& device, VkRenderPass renderPass, std::string vertexShader, std::string fragmentShader, VkDescriptorImageInfo imageInfo);
	virtual ~ShadowRenderSystem();

	VkDescriptorSetLayout getLightSetLayout() const { return lightSetLayout->getDescriptorSetLayout(); }
	VkDescriptorSet getLightSet(int frameIndex) const { return lightSets[frameIndex]; }

	void setupDescriptorSet(ShDescriptorPool& pool);
	void setupLight(const ShGameObject& light, int frameIndex);

	virtual void createPipeline(VkRenderPass renderPass) override;
	virtual void renderGameObjects(FrameInfo& frameInfo) override;
};

