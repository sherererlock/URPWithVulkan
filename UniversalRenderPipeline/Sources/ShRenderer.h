#pragma once
#include <cassert>

#include "ShSwapchain.h"

class ShRenderer
{
private:
	ShWindow& shWindow;
	ShDevice& shDevice;

	std::unique_ptr<ShSwapchain> shSwapchain;
	std::vector<VkCommandBuffer> commandBuffers;

	uint32_t currentImageIndex = 0;
	int currentFrameIndex{ 0 };
	bool isFrameStarted{ false };

	void createCommandBuffers();
	void freeCommandBuffers();
	void recreateSwapChain();

public:
	ShRenderer(ShWindow& window, ShDevice& device);
	~ShRenderer();

	ShRenderer(const ShRenderer&) = delete;
	ShRenderer& operator=(const ShRenderer&) = delete;

	VkRenderPass getSwapChainRenderPass() const { return shSwapchain->getRenderPass(); }
	VkFormat getFormat() const { return shSwapchain->getSwapChainImageFormat(); }
	float getAspectRatio() const { return shSwapchain->extentAspectRatio(); }
	bool isFrameInProgress() const { return isFrameStarted; }

	VkCommandBuffer getCurrentCommandBuffer() const {
		//assert(isFrameStarted && "Cannot get command buffer when frame not in progress");
		return commandBuffers[currentFrameIndex];
	}

	int getFrameIndex() const {
		assert(isFrameStarted && "Cannot get frame index when frame not in progress");
		return currentFrameIndex;
	}

	void beginFrame();
	void endFrame();
	VkCommandBuffer beginCommandBuffer();
	void endCommandBuffer();
	void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
	void endSwapChainRenderPass(VkCommandBuffer commandBuffer);
};

