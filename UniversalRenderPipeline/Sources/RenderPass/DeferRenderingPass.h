#pragma once
#include "ShRenderPass.h"
class DeferRenderingPass : public ShRenderPass
{
private: 
    FrameBufferAttachment color;
    FrameBufferAttachment albedo;
    FrameBufferAttachment normal;
    FrameBufferAttachment emissive;
    FrameBufferAttachment position;
    FrameBufferAttachment depth;

protected:
    // Í¨¹ý ShRenderPass ¼Ì³Ð
    std::vector<VkAttachmentDescription> GetAttachmentDescriptions() const override;
    std::vector<VkSubpassDescription> GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const override;
    std::vector<VkSubpassDependency> GetSubpassDependencies() const override;
    std::vector<VkAttachmentReference> GetColorAttachmentRefs() const override;
    VkAttachmentReference GetDepthAttachmentRef() const override;
    std::vector<VkImageView> GetImageViews() const override;
    std::vector<VkClearValue> GetClearValues() const override;
public:

    DeferRenderingPass(ShDevice& device, uint32_t w, uint32_t h, VkFormat format, VkFormat dformat);

    virtual void createRenderPass() override;

    VkDescriptorImageInfo GetColor() const { return color.descriptor(); }
    VkDescriptorImageInfo GetAlbedo() const { return albedo.descriptor(); }
    VkDescriptorImageInfo GetNormal() const { return normal.descriptor(); }
    VkDescriptorImageInfo GetEmissive() const { return emissive.descriptor(); }
    VkDescriptorImageInfo GetPosition() const { return position.descriptor(); }
    VkDescriptorImageInfo GetDepth() const { return depth.descriptor(); }
};

