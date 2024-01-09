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

	~FrameBufferAttachment()
	{
		vkDestroyImage(device, image, nullptr);
		vkDestroyImageView(device, view, nullptr);
		vkFreeMemory(device, mem, nullptr);
		vkDestroySampler(device, sampler, nullptr);
	}

	VkDescriptorImageInfo descriptor()
	{
		return VkDescriptorImageInfo{ sampler, view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
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
	virtual std::vector<VkSubpassDescription> GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs) const = 0;
	virtual std::vector<VkSubpassDependency> GetSubpassDependencies() const = 0;
	virtual std::vector<VkAttachmentReference> GetAttachmentRefs() const = 0;
	virtual std::vector<VkImageView> GetImageViews() const = 0;
	virtual std::vector<VkClearValue> GetClearValues() const = 0;

public:
	ShRenderPass(ShDevice& device, uint32_t w, uint32_t h);
	virtual ~ShRenderPass();

	VkRenderPass getRenderPass() const { return renderpass; }

	void createRenderPass();
	void createFrameBuffer();

	void beginRenderPass(VkCommandBuffer commandBuffer);
	void endRenderPass(VkCommandBuffer commandBuffer);

	void createAttachment(FrameBufferAttachment* attachment, VkFormat format, VkImageUsageFlags usage);
};

