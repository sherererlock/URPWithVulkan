#include<array>

#include "LightingPass.h"

std::vector<VkAttachmentDescription> LightingPass::GetAttachmentDescriptions() const
{
    VkAttachmentDescription colorAttachmentDes{};

    colorAttachmentDes.format = colorFormat;
    colorAttachmentDes.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachmentDes.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachmentDes.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachmentDes.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachmentDes.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachmentDes.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachmentDes.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    return { colorAttachmentDes };
}

std::vector<VkSubpassDescription> LightingPass::GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const
{
    VkSubpassDescription subpassDes{};

    uint32_t colorCount = (uint32_t)(attachmentRefs.size());
    subpassDes.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDes.colorAttachmentCount = colorCount;
    subpassDes.pColorAttachments = attachmentRefs.data();
    subpassDes.pDepthStencilAttachment = VK_NULL_HANDLE;

    return { subpassDes };
}

std::vector<VkSubpassDependency> LightingPass::GetSubpassDependencies() const
{
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    return { dependencies[0], dependencies[1]};
}

std::vector<VkAttachmentReference> LightingPass::GetColorAttachmentRefs() const
{
    VkAttachmentReference ref;
    ref.attachment = 0;
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    return { ref };
}

std::vector<VkImageView> LightingPass::GetImageViews() const
{
    return { color.view };
}

std::vector<VkClearValue> LightingPass::GetClearValues() const
{
    VkClearValue clearValue{};
    clearValue.color = { 0, 0, 0, 1 };
    return { clearValue };
}

LightingPass::LightingPass(ShDevice& device, uint32_t w, uint32_t h, VkFormat format)
    : ShRenderPass(device, w, h), colorFormat(format)
{
    createAttachment(&color, colorFormat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    createRenderPass();
    createFrameBuffer();
}

LightingPass::~LightingPass()
{
}
