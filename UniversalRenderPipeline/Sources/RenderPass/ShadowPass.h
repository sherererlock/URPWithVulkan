#pragma once
#include "ShRenderPass.h"
class ShadowPass : public ShRenderPass
{
private:
	FrameBufferAttachment shadowMap;

protected:
	// ͨ�� ShRenderPass �̳�
	std::vector<VkAttachmentDescription> GetAttachmentDescriptions() const override;
	std::vector<VkSubpassDescription> GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const override;
	std::vector<VkSubpassDependency> GetSubpassDependencies() const override;
	std::vector<VkAttachmentReference> GetColorAttachmentRefs() const override;
	VkAttachmentReference GetDepthAttachmentRef() const override;
	std::vector<VkImageView> GetImageViews() const override;
	std::vector<VkClearValue> GetClearValues() const override;

public:
	ShadowPass(ShDevice& device, uint32_t w, uint32_t h);
	virtual ~ShadowPass();

	VkDescriptorImageInfo getShadowMapImageInfo() { return shadowMap.descriptor(); }
};

