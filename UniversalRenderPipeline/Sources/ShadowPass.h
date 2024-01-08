#pragma once
#include "ShRenderPass.h"
class ShadowPass : public ShRenderPass
{
private:
	FrameBufferAttachment shadowMap;

protected:
	// Í¨¹ý ShRenderPass ¼Ì³Ð
	std::vector<VkAttachmentDescription> GetAttachmentDescriptions() const override;
	std::vector<VkSubpassDescription> GetSubpassDescriptions(const std::vector<VkAttachmentReference> attachmentRefs) const override;
	std::vector<VkSubpassDependency> GetSubpassDependencies() const override;
	std::vector<VkAttachmentReference> GetAttachmentRefs() const override;
	std::vector<VkImageView> GetImageViews() const override;
	std::vector<VkClearValue> GetClearValues() const override;

public:
	ShadowPass(ShDevice& device, uint32_t w, uint32_t h);
	virtual ~ShadowPass();
};

