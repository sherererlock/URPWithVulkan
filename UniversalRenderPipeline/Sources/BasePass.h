#pragma once
#include "ShRenderPass.h"
class BasePass : public ShRenderPass
{
private:
	FrameBufferAttachment albedo;
	FrameBufferAttachment normal;
	FrameBufferAttachment emissive;
	FrameBufferAttachment depth;

protected:
	// Í¨¹ý ShRenderPass ¼Ì³Ð
	std::vector<VkAttachmentDescription> GetAttachmentDescriptions() const override;
	std::vector<VkSubpassDescription> GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs) const override;
	std::vector<VkSubpassDependency> GetSubpassDependencies() const override;
	std::vector<VkAttachmentReference> GetAttachmentRefs() const override;
	std::vector<VkImageView> GetImageViews() const override;
	std::vector<VkClearValue> GetClearValues() const override;

public:

	BasePass(ShDevice& device, uint32_t w, uint32_t h);
	~BasePass() {};

	VkDescriptorImageInfo GetAlbedo() const { return albedo.descriptor(); }
	VkDescriptorImageInfo GetNormal() const { return normal.descriptor(); }
	VkDescriptorImageInfo GetEmissive() const { return emissive.descriptor(); }
	VkDescriptorImageInfo GetDepth() const { return depth.descriptor(); }
};

