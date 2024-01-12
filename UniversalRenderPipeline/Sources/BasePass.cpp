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

    VkFormat format = shDevice.findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    VkAttachmentDescription depthAttachmentDes;
    depthAttachmentDes.format = format;
    depthAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    return { colorAttachmentDes, colorAttachmentDes, colorAttachmentDes, depthAttachmentDes };
}

std::vector<VkSubpassDescription> BasePass::GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs) const
{
    VkSubpassDescription subpassDes;

    uint32_t colorCount = (uint32_t)(attachmentRefs.size() - 1);
    subpassDes.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDes.colorAttachmentCount = colorCount;
    subpassDes.pColorAttachments = attachmentRefs.data();
    subpassDes.pDepthStencilAttachment = &attachmentRefs[colorCount];

    return {subpassDes};
}

std::vector<VkSubpassDependency> BasePass::GetSubpassDependencies() const
{
    std::array<VkSubpassDependency, 1> dependencies;

    dependencies[0].srcSubpass = 0;
    dependencies[0].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    return { dependencies[0] };
}

std::vector<VkAttachmentReference> BasePass::GetAttachmentRefs() const
{
    std::vector<VkAttachmentReference> colorRefs(4);

    for (int i = 0; i < colorRefs.size(); i++)
    {
        colorRefs[i].attachment = i;
        colorRefs[i].layout = i == int(colorRefs.size() - 1) ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    return colorRefs;
}

std::vector<VkImageView> BasePass::GetImageViews() const
{
    return {albedo.view, normal.view, emissive.view, depth.view};
}

std::vector<VkClearValue> BasePass::GetClearValues() const
{
    std::vector<VkClearValue> clearValues(4);
    for (VkClearValue& clearValue : clearValues)
    {
        clearValue.color = { 0.0f, 0.0f, 0.0f, 1.0f };
        clearValue.depthStencil = { 1.0f, 0 };
    }

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
