#pragma once
#include "ShRenderPass.h"

class LightingPass : public ShRenderPass
{
private:
	VkFormat colorFormat;
	FrameBufferAttachment color;


protected:
	// Í¨¹ý ShRenderPass ¼Ì³Ð
	std::vector<VkAttachmentDescription> GetAttachmentDescriptions() const override;
	std::vector<VkSubpassDescription> GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const override;
	std::vector<VkSubpassDependency> GetSubpassDependencies() const override;
	std::vector<VkAttachmentReference> GetColorAttachmentRefs() const override;
	std::vector<VkImageView> GetImageViews() const override;
	std::vector<VkClearValue> GetClearValues() const override;

public:

	LightingPass(ShDevice& device, uint32_t w, uint32_t h, VkFormat colorFormat);
	~LightingPass();

	VkDescriptorImageInfo getColor() const { return color.descriptor(); }
};

