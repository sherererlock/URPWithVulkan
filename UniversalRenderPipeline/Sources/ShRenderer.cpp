
#include <stdexcept>
#include <array>
#include <cassert>

#include "ShRenderer.h"

void ShRenderer::createCommandBuffers()
{
    commandBuffers.resize(ShSwapchain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = shDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    // initial state
    if (vkAllocateCommandBuffers(shDevice.device(), &allocInfo, commandBuffers.data()) !=
        VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void ShRenderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(
        shDevice.device(),
        shDevice.getCommandPool(),
        static_cast<uint32_t>(commandBuffers.size()),
        commandBuffers.data());
    commandBuffers.clear();
}

void ShRenderer::recreateSwapChain()
{
    auto extent = shWindow.getExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = shWindow.getExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(shDevice.device());

    if (shSwapchain == nullptr)
    {
        shSwapchain = std::make_unique<ShSwapchain>(shDevice, extent);
    }
    else
    {
        std::shared_ptr<ShSwapchain> oldSwapChain = std::move(shSwapchain);
        shSwapchain = std::make_unique<ShSwapchain>(shDevice, extent, oldSwapChain);

        if (!oldSwapChain->compareSwapFormats(*shSwapchain.get()))
        {
            throw std::runtime_error("Swap chain image(or depth) format has changed!");
        }
    }
}

ShRenderer::ShRenderer(ShWindow& window, ShDevice& device)
    : shWindow(window), shDevice(device)
{
    recreateSwapChain();
    createCommandBuffers();
}

ShRenderer::~ShRenderer()
{
    freeCommandBuffers();
}

void ShRenderer::beginFrame()
{
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = shSwapchain->acquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    isFrameStarted = true;
}

void ShRenderer::endFrame()
{
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    // Pending state
    shSwapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    auto result = shSwapchain->submitFrame(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        shWindow.wasWindowResized()) 
    {
        shWindow.resetWindowResizedFlag();
        recreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    isFrameStarted = false;
    currentFrameIndex = (currentFrameIndex + 1) % ShSwapchain::MAX_FRAMES_IN_FLIGHT;
    vkQueueWaitIdle(shDevice.presentQueue());
}

VkCommandBuffer ShRenderer::beginCommandBuffer()
{
    if (!isFrameStarted)
        return VK_NULL_HANDLE;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    //beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    // Recording state
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    return commandBuffer;
}

void ShRenderer::endCommandBuffer()
{
    // Executable state
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void ShRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = shSwapchain->getRenderPass();
	renderPassInfo.framebuffer = shSwapchain->getFrameBuffer(currentImageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = shSwapchain->getSwapChainExtent();

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(shSwapchain->getSwapChainExtent().width);
	viewport.height = static_cast<float>(shSwapchain->getSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0, 0}, shSwapchain->getSwapChainExtent() };
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void ShRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
	assert(isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
	assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
	vkCmdEndRenderPass(commandBuffer);
}
