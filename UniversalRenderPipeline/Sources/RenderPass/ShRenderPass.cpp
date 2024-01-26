#include<stdexcept>

#include "ShRenderPass.h"
#include "VulkanInitializers.hpp"

ShRenderPass::ShRenderPass(ShDevice& device, uint32_t w, uint32_t h) 
	: shDevice(device), width(w), height(h)
{
}

ShRenderPass::~ShRenderPass()
{
	vkDestroyRenderPass(shDevice.device(), renderpass, nullptr);
	vkDestroyFramebuffer(shDevice.device(), framebuffer, nullptr);
}

void ShRenderPass::createRenderPass()
{
    auto attachments = GetAttachmentDescriptions();
    auto attachmentRefs = GetColorAttachmentRefs();
    auto depthRef = GetDepthAttachmentRef();
    auto subpassdes = GetSubpassDescriptions(attachmentRefs, depthRef);
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

void ShRenderPass::createFrameBuffer()
{
    auto attachments = GetImageViews();

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderpass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(shDevice.device(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void ShRenderPass::beginRenderPass(VkCommandBuffer commandBuffer)
{
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderpass;
    renderPassInfo.framebuffer = framebuffer;

    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = {width, height};
    auto clearValues = GetClearValues();
    renderPassInfo.clearValueCount = (uint32_t) clearValues.size();
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{ {0, 0}, {width, height} };
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void ShRenderPass::endRenderPass(VkCommandBuffer commandBuffer)
{
    vkCmdEndRenderPass(commandBuffer);
}

void ShRenderPass::nextSubPass(VkCommandBuffer commandBuffer)
{
    vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);
}

void ShRenderPass::createAttachment(FrameBufferAttachment* attachment, VkFormat format, VkImageUsageFlags usage, VkImageLayout imageLayout)
{
    attachment->format = format;
    attachment->device = shDevice.device();
    attachment->layout = imageLayout;

    VkImageAspectFlags aspectMask = 0;
    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
    {
        aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
    {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        if (format >= VK_FORMAT_D16_UNORM_S8_UINT)
            aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    VkImageCreateInfo imageInfo = Initializers::imageCreateInfo();

    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = format;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    shDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment->image, attachment->mem);
    shDevice.createImageView(attachment->view, attachment->image, format, aspectMask, 1);
    if(usage & VK_IMAGE_USAGE_SAMPLED_BIT)
        shDevice.createSampler(attachment->sampler, VK_FILTER_NEAREST, VK_FILTER_NEAREST, 1, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
}
