#pragma once
#include "ShDevice.h"
#include<vulkan/vulkan.h>

struct FrameBufferAttachment
{
	VkImage image;
	VkDeviceMemory mem;
	VkImageView view;
	VkFormat format;
	VkSampler sampler;
	VkDevice device;
	VkImageLayout layout;

	~FrameBufferAttachment()
	{
		vkDestroyImage(device, image, nullptr);
		vkDestroyImageView(device, view, nullptr);
		vkFreeMemory(device, mem, nullptr);
		vkDestroySampler(device, sampler, nullptr);
	}

	VkDescriptorImageInfo descriptor() const
	{
		return VkDescriptorImageInfo{ sampler, view, layout };
	}
};

class ShRenderPass
{
protected:
	ShDevice& shDevice;
	uint32_t width = 0, height = 0;
	VkRenderPass renderpass{VK_NULL_HANDLE};
	VkFramebuffer framebuffer{ VK_NULL_HANDLE };

protected:
	virtual std::vector<VkAttachmentDescription> GetAttachmentDescriptions() const = 0 ;
	virtual std::vector<VkSubpassDescription> GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const = 0;
	virtual std::vector<VkSubpassDependency> GetSubpassDependencies() const = 0;
	virtual std::vector<VkAttachmentReference> GetColorAttachmentRefs() const = 0;
	virtual VkAttachmentReference GetDepthAttachmentRef() const { return VkAttachmentReference{}; }
	virtual std::vector<VkImageView> GetImageViews() const = 0;
	virtual std::vector<VkClearValue> GetClearValues() const = 0;

	void createAttachment(FrameBufferAttachment* attachment, VkFormat format, VkImageUsageFlags usage, VkImageLayout imageLayout);

public:
	ShRenderPass(ShDevice& device, uint32_t w, uint32_t h);
	virtual ~ShRenderPass();

	VkRenderPass getRenderPass() const { return renderpass; }

	virtual void createRenderPass();
	void createFrameBuffer();

	void beginRenderPass(VkCommandBuffer commandBuffer);
	void endRenderPass(VkCommandBuffer commandBuffer);

	void nextSubPass(VkCommandBuffer commandBuffer);

};

