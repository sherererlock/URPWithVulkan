
#include <stdexcept>

#include "ShRenderer.h"

void ShRenderer::createCommandBuffers()
{
    commandBuffers.resize(ShSwapchain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = shDevice.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

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

VkCommandBuffer ShRenderer::beginFrame()
{
    assert(!isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = shSwapchain->acquireNextImage(&currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return nullptr;
    }

    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    isFrameStarted = true;

    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    return commandBuffer;
}

void ShRenderer::endFrame()
{
    assert(isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    auto result = shSwapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
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
}

void ShRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
}

void ShRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
}
