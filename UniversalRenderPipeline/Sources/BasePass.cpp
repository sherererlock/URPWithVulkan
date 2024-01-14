#include "BasePass.h"
#include <array>

std::vector<VkAttachmentDescription> BasePass::GetAttachmentDescriptions() const
{
    VkAttachmentDescription colorAttachmentDes{};

    colorAttachmentDes.format = VK_FORMAT_R8G8B8A8_UNORM;
    colorAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentDescription depthAttachmentDes{};
    depthAttachmentDes.format = depth.format;
    depthAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    return { colorAttachmentDes, colorAttachmentDes, colorAttachmentDes, depthAttachmentDes };
}

std::vector<VkSubpassDescription> BasePass::GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const
{
    VkSubpassDescription subpassDes{};

    subpassDes.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDes.colorAttachmentCount = (uint32_t)(attachmentRefs.size());
    subpassDes.pColorAttachments = attachmentRefs.data();
    subpassDes.pDepthStencilAttachment = &depthRef;

    return {subpassDes};
}

std::vector<VkSubpassDependency> BasePass::GetSubpassDependencies() const
{
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    return { dependencies[0], dependencies[1] };
}

std::vector<VkAttachmentReference> BasePass::GetColorAttachmentRefs() const
{
    std::vector<VkAttachmentReference> colorRefs(3);

    for (int i = 0; i < colorRefs.size(); i++)
    {
        colorRefs[i].attachment = i;
        colorRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    return colorRefs;
}

VkAttachmentReference BasePass::GetDepthAttachmentRef() const
{
    VkAttachmentReference depthRef;

    depthRef.attachment = 3;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return depthRef;
}

std::vector<VkImageView> BasePass::GetImageViews() const
{
    return {albedo.view, normal.view, emissive.view, depth.view};
}

std::vector<VkClearValue> BasePass::GetClearValues() const
{
    std::vector<VkClearValue> clearValues(4);
    for (VkClearValue& clearValue : clearValues)
        clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

    clearValues[3].depthStencil = { 1.0f, 0 };

    return clearValues;
}

BasePass::BasePass(ShDevice& device, uint32_t w, uint32_t h) 
    : ShRenderPass(device, w, h)
{
    createAttachment(&albedo, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    createAttachment(&normal, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    createAttachment(&emissive, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkFormat format = device.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    createAttachment(&depth, format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    createRenderPass();
    createFrameBuffer();
}
