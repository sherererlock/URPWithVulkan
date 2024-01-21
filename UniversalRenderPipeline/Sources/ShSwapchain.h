#pragma once

#include "ShDevice.h"

#include<vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

class ShSwapchain
{
private:
	void init();
	void createSwapChain();
	void createImageViews();
	void createDepthResources();
	void createRenderPass();
	void createFramebuffers();
	void createSyncObjects();

	// Helper functions
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	VkFormat swapChainImageFormat;
	VkFormat swapChainDepthFormat;
	VkExtent2D swapChainExtent;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;

	ShDevice& device;
	VkExtent2D windowExtent;

	VkSwapchainKHR swapChain;
	std::shared_ptr<ShSwapchain> oldSwapChain;
	VkRenderPass renderPass;
	std::vector<VkFramebuffer> swapChainFramebuffers;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	std::vector<VkFence> imagesInFlight;
	size_t currentFrame = 0;


public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	ShSwapchain(ShDevice& device, VkExtent2D windowExtent);
	ShSwapchain(
		ShDevice& deviceRef, VkExtent2D windowExtent, std::shared_ptr<ShSwapchain> previous);

	~ShSwapchain();

	ShSwapchain(const ShSwapchain&) = delete;
	ShSwapchain& operator=(const ShSwapchain&) = delete;

	VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
	VkRenderPass getRenderPass() { return renderPass; }
	VkImageView getImageView(int index) { return swapChainImageViews[index]; }
	size_t imageCount() { return swapChainImages.size(); }
	VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
	VkExtent2D getSwapChainExtent() { return swapChainExtent; }
	uint32_t width() { return swapChainExtent.width; }
	uint32_t height() { return swapChainExtent.height; }

	VkFormat findDepthFormat();
	VkResult acquireNextImage(uint32_t* imageIndex);
	void submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
	VkResult submitFrame(uint32_t* imageIndex);

	float extentAspectRatio()
	{
		return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
	}

	bool compareSwapFormats(const ShSwapchain& swapChain) const {
		return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
			swapChain.swapChainImageFormat == swapChainImageFormat;
	}
};

