#include<stdexcept>
#include<array>
#include "DeferRenderingPass.h"

std::vector<VkAttachmentDescription> DeferRenderingPass::GetAttachmentDescriptions() const
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

#ifdef CALC_POSITION
    std::vector<VkAttachmentDescription> attachments = { colorAttachmentDes, colorAttachmentDes, colorAttachmentDes, depthAttachmentDes };
#else
    std::vector<VkAttachmentDescription> attachments = { colorAttachmentDes, colorAttachmentDes, colorAttachmentDes, colorAttachmentDes, colorAttachmentDes, depthAttachmentDes };
    attachments[4].format = VK_FORMAT_R16G16B16A16_SFLOAT;    
#endif // CALC_POSITION

    attachments[0].format = color.format;
    return attachments;;
}

std::vector<VkSubpassDescription> DeferRenderingPass::GetSubpassDescriptions(const std::vector<VkAttachmentReference>& attachmentRefs, const VkAttachmentReference& depthRef) const
{
    VkSubpassDescription subpassDes{};

    subpassDes.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDes.colorAttachmentCount = (uint32_t)(attachmentRefs.size());
    subpassDes.pColorAttachments = attachmentRefs.data();
    subpassDes.pDepthStencilAttachment = &depthRef;

    return { subpassDes };
}

std::vector<VkSubpassDependency> DeferRenderingPass::GetSubpassDependencies() const
{
    std::array<VkSubpassDependency, 5> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = 0;

    dependencies[1].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].dstSubpass = 0;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].srcAccessMask = 0;
    dependencies[1].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dependencyFlags = 0;

    dependencies[2].srcSubpass = 0;
    dependencies[2].dstSubpass = 1;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[3].srcSubpass = 1;
    dependencies[3].dstSubpass = 2;
    dependencies[3].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[3].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[3].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[3].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
    dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[4].srcSubpass = 2;
    dependencies[4].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[4].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[4].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[4].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[4].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[4].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    return { dependencies[0], dependencies[1], dependencies[2], dependencies[3], dependencies[4] };
}

std::vector<VkAttachmentReference> DeferRenderingPass::GetColorAttachmentRefs() const
{
#ifdef CALC_POSITION
    std::vector<VkAttachmentReference> colorRefs(4);
#else
    std::vector<VkAttachmentReference> colorRefs(5);
#endif

    for (int i = 0; i < colorRefs.size(); i++)
    {
        colorRefs[i].attachment = i;
        colorRefs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }

    return colorRefs;
}

VkAttachmentReference DeferRenderingPass::GetDepthAttachmentRef() const
{
    VkAttachmentReference depthRef;

#ifdef CALC_POSITION
    depthRef.attachment = 4;
#else
    depthRef.attachment = 5;
#endif
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return depthRef;
}

std::vector<VkImageView> DeferRenderingPass::GetImageViews() const
{
#ifdef CALC_POSITION
    return { color.view, albedo.view, normal.view, emissive.view, depth.view };
#else

    return { color.view, albedo.view, normal.view, emissive.view, position.view, depth.view };
#endif
}

std::vector<VkClearValue> DeferRenderingPass::GetClearValues() const
{
#ifdef CALC_POSITION

    std::vector<VkClearValue> clearValues(5);
    for (VkClearValue& clearValue : clearValues)
        clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

    clearValues[4].depthStencil = { 1.0f, 0 };

#else
    std::vector<VkClearValue> clearValues(6);
    for (VkClearValue& clearValue : clearValues)
        clearValue.color = { 0.0f, 0.0f, 0.0f, 0.0f };

    clearValues[5].depthStencil = { 1.0f, 0 };
#endif

    return clearValues;
}

DeferRenderingPass::DeferRenderingPass(ShDevice& device, uint32_t w, uint32_t h, VkFormat cformat, VkFormat dformat)
    : ShRenderPass(device, w, h)
{
    createAttachment(&color, cformat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT , VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    createAttachment(&albedo, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    createAttachment(&normal, VK_FORMAT_R8G8B8A8_UNORM,  VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    createAttachment(&emissive, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    createAttachment(&position, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    createAttachment(&depth, dformat, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL);

    createRenderPass();
    createFrameBuffer();
}

void DeferRenderingPass::createRenderPass()
{
    auto attachments = GetAttachmentDescriptions();
    auto attachmentRefs = GetColorAttachmentRefs();
    auto depthRef = GetDepthAttachmentRef();
    auto subpassdes0 = GetSubpassDescriptions(attachmentRefs, depthRef);

    VkAttachmentReference colorRef = attachmentRefs[0];
    
#ifdef CALC_POSITION
    std::vector<VkAttachmentReference> inputRefs = { attachmentRefs[1] , attachmentRefs[2], attachmentRefs[3] };
    std::vector<VkAttachmentReference> inputRefs1 = { };
#else
    std::vector<VkAttachmentReference> inputRefs = { attachmentRefs[1] , attachmentRefs[2], attachmentRefs[3], attachmentRefs[4] };
    std::vector<VkAttachmentReference> inputRefs1 = { attachmentRefs[4] };
#endif
    for (int i = 0; i < inputRefs.size(); i++)
    {
        inputRefs[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    for (int i = 0; i < inputRefs1.size(); i++)
    {
        inputRefs1[i].layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }

    VkSubpassDescription subpassDes1{};
    subpassDes1.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDes1.colorAttachmentCount = 1;
    subpassDes1.pColorAttachments = &colorRef;
    subpassDes1.pDepthStencilAttachment = &depthRef;
    subpassDes1.inputAttachmentCount = (uint32_t)inputRefs.size();
    subpassDes1.pInputAttachments = inputRefs.data();

    VkSubpassDescription subpassDes2{};
    subpassDes2.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDes2.colorAttachmentCount = 1;
    subpassDes2.pColorAttachments = &colorRef;
    subpassDes2.pDepthStencilAttachment = &depthRef;
    subpassDes2.inputAttachmentCount = (uint32_t)inputRefs1.size();
    subpassDes2.pInputAttachments = inputRefs1.data();

    std::array<VkSubpassDescription, 3> subpassdes = { subpassdes0[0], subpassDes1, subpassDes2 };

    auto subpassdens = GetSubpassDependencies();

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = static_cast<uint32_t>(subpassdes.size());
    renderPassInfo.pSubpasses = subpassdes.data();

    renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassdens.size());
    renderPassInfo.pDependencies = subpassdens.data();

    if (vkCreateRenderPass(shDevice.device(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create render pass!");
    }
}