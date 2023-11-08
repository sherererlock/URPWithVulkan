#include<vector>
#include <array>
#include <string>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <set>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm/glm.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/gtx/hash.hpp>

const int width = 1280;
const int height = 720;

struct QueueFamilyIndices
{
	int graphicsFamily = -1;
	int presentFamily = -1;

	bool isComplete() {
		return graphicsFamily >= 0 && presentFamily >= 0;
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities = {};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

struct PipelineCreateInfo
{
	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo inputAssembly;
	VkPipelineViewportStateCreateInfo viewportState;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkPipelineMultisampleStateCreateInfo multisampling;
	VkPipelineColorBlendStateCreateInfo colorBlending;
	VkPipelineDynamicStateCreateInfo dynamicState;
	VkPipelineDepthStencilStateCreateInfo depthStencil;

	PipelineCreateInfo() : vertexInputInfo({}), inputAssembly({}), viewportState({}), rasterizer({}),
		multisampling({}), colorBlending({}), dynamicState({}), depthStencil({})
	{
	}

	void Apply(VkGraphicsPipelineCreateInfo& createInfo) const
	{
		createInfo.pVertexInputState = &vertexInputInfo; // bindings and attribute
		createInfo.pInputAssemblyState = &inputAssembly; // topology
		createInfo.pViewportState = &viewportState;
		createInfo.pRasterizationState = &rasterizer;
		createInfo.pMultisampleState = &multisampling;
		createInfo.pDepthStencilState = &depthStencil; // Optional
		createInfo.pColorBlendState = &colorBlending;
		createInfo.pDynamicState = &dynamicState; // Optional
	}
};

struct VulkanInfo
{
	GLFWwindow* window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;

	VkSurfaceKHR surface;
	VkPhysicalDevice physicsDevice;
	VkDevice device;

	VkQueue graphicsQueue;
	VkQueue presentQueue;

	VkSwapchainKHR swapChain;

	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	std::vector<VkCommandBuffer> commandBuffers;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkRenderPass renderPass;

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	VkDescriptorPool descriptorPool;
	VkDescriptorSetLayout descriptorSetLayout;

	VkBuffer uniformBuffer;
	VkDeviceMemory uniformBufferMemory;

	VkCommandPool commandPool;
	VkSurfaceFormatKHR swapChainImageFormat = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
	VkExtent2D swapChainExtent = {width, height};
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT  messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

std::vector<const char*> GetRequiredExtensions()
{
	std::vector<const char*> extensions;

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; i++)
		extensions.push_back(glfwExtensions[i]);

	extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void CreateVkInstance(VulkanInfo& vulkanInfo)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan Test";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// 对整个程序都有效的扩展和layer
	// VK_KHR_Surface
	// VK_KHR_win32_surface
	// VK_EXT_debug_utils
	auto extensions = GetRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	debugCreateInfo = {};
	debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	debugCreateInfo.pfnUserCallback = debugCallback;

	createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;

	if (vkCreateInstance(&createInfo, nullptr, &vulkanInfo.instance) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create instance!");
	}

	VkDebugUtilsMessengerCreateInfoEXT DebugUtilCreateInfo;
	DebugUtilCreateInfo = {};
	DebugUtilCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	DebugUtilCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	DebugUtilCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	DebugUtilCreateInfo.pfnUserCallback = debugCallback;

	if (CreateDebugUtilsMessengerEXT(vulkanInfo.instance, &DebugUtilCreateInfo, nullptr, &vulkanInfo.debugMessenger) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VkPhysicalDevice pickPhysicalDevice(VkInstance instance)
{
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
	for (const auto& phydevice : devices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(phydevice, &deviceProperties);

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures(phydevice, &deviceFeatures);

		std::cout << deviceProperties.deviceName << std::endl;
		physicalDevice = phydevice;
		break;
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}

	return physicalDevice;
}

QueueFamilyIndices findQueueFamilies(const VulkanInfo& vulkanInfo)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(vulkanInfo.physicsDevice, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(vulkanInfo.physicsDevice, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(vulkanInfo.physicsDevice, i, vulkanInfo.surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport)
		{
			indices.presentFamily = i;
		}

		if (indices.isComplete())
		{
			break;
		}

		i++;
	}

	return indices;
}

void CreateDevice(VulkanInfo& vulkanInfo)
{
	QueueFamilyIndices indices = findQueueFamilies(vulkanInfo);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	deviceFeatures.sampleRateShading = VK_TRUE;
	deviceFeatures.fillModeNonSolid = VK_TRUE;
	deviceFeatures.imageCubeArray = VK_TRUE;

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	createInfo.pEnabledFeatures = &deviceFeatures;

	// 适配于device的extension
	// VK_KHR_swapchain
	const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	createInfo.ppEnabledLayerNames = validationLayers.data();

	if (vkCreateDevice(vulkanInfo.physicsDevice, &createInfo, nullptr, &vulkanInfo.device) != VK_SUCCESS)
		throw std::runtime_error("failed to create logical device!");

	vkGetDeviceQueue(vulkanInfo.device, indices.graphicsFamily, 0, &vulkanInfo.graphicsQueue);
	vkGetDeviceQueue(vulkanInfo.device, indices.presentFamily, 0, &vulkanInfo.presentQueue);
}

SwapChainSupportDetails QuerySwapChainSupport(const VulkanInfo& vulkanInfo)
{
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanInfo.physicsDevice, vulkanInfo.surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanInfo.physicsDevice, vulkanInfo.surface, &formatCount, nullptr);

	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanInfo.physicsDevice, vulkanInfo.surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanInfo.physicsDevice, vulkanInfo.surface, &presentModeCount, nullptr);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanInfo.physicsDevice, vulkanInfo.surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

uint32_t FindMemoryType(const VulkanInfo& vulkanInfo, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vulkanInfo.physicsDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if (typeFilter & (1 << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");

	return uint32_t();
}

void CreateImage(const VulkanInfo& vulkanInfo, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t miplevels, VkSampleCountFlagBits numSamples, uint32_t layers = 1, VkImageCreateFlags flag = 0)
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(width);
	imageInfo.extent.height = static_cast<uint32_t>(height);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = miplevels;
	imageInfo.arrayLayers = layers;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = numSamples;
	imageInfo.flags = flag; // Optional

	if (vkCreateImage(vulkanInfo.device, &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(vulkanInfo.device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(vulkanInfo, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(vulkanInfo.device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(vulkanInfo.device, image, imageMemory, 0);
}

void CreateImageView(VkDevice device, VkImageView& view, VkImage& image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t miplevels, VkImageViewType viewtype = VK_IMAGE_VIEW_TYPE_2D, uint32_t layers = 1)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = viewtype;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = miplevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = layers;

	if (vkCreateImageView(device, &viewInfo, nullptr, &view) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create texture image view!");
	}
}

VkCommandBuffer BeginSingleTimeCommands(const VulkanInfo& vulkanInfo)
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = vulkanInfo.commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(vulkanInfo.device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void EndSingleTimeCommands(const VulkanInfo& vulkanInfo, VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = 0;

	VkFence fence;
	vkCreateFence(vulkanInfo.device, &fenceCreateInfo, nullptr, &fence);

	vkQueueSubmit(vulkanInfo.graphicsQueue, 1, &submitInfo, fence);

	vkWaitForFences(vulkanInfo.device, 1, &fence, VK_TRUE, 1000);
	vkDestroyFence(vulkanInfo.device, fence, nullptr);

	vkFreeCommandBuffers(vulkanInfo.device, vulkanInfo.commandPool, 1, &commandBuffer);
}

void TransitionImageLayout(const VulkanInfo& vulkanInfo, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t miplevels)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands(vulkanInfo);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;

	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = miplevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
	{
		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else
	{
		throw std::invalid_argument("unsupported layout transition!");
	}

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL || format == vulkanInfo.depthFormat)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT)
		{
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands(vulkanInfo, commandBuffer);
}

void CreateSwapChainAndImageView(VulkanInfo& vulkanInfo)
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(vulkanInfo);

	// surface的format,包括图像的colorspace和format
	// presentmode 三缓冲
	// extent 图像分辨率
	// 图像的数量
	// 队列对于图像的使用：呈现队列和渲染队列

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = vulkanInfo.surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = vulkanInfo.swapChainImageFormat.format;
	createInfo.imageColorSpace = vulkanInfo.swapChainImageFormat.colorSpace;
	createInfo.imageExtent = vulkanInfo.swapChainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(vulkanInfo);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = vulkanInfo.presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(vulkanInfo.device, &createInfo, nullptr, &vulkanInfo.swapChain) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create swap chain!");
	}

	// 获取交换链中的vkImage

	vkGetSwapchainImagesKHR(vulkanInfo.device, vulkanInfo.swapChain, &imageCount, nullptr);
	vulkanInfo.swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vulkanInfo.device, vulkanInfo.swapChain, &imageCount, vulkanInfo.swapChainImages.data());

	vulkanInfo.swapChainImageViews.resize(vulkanInfo.swapChainImages.size());
	for (size_t i = 0; i < vulkanInfo.swapChainImages.size(); i++)
	{
		CreateImageView(vulkanInfo.device, vulkanInfo.swapChainImageViews[i], vulkanInfo.swapChainImages[i], vulkanInfo.swapChainImageFormat.format, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_2D, 1);
	}
}

void CreateDepthResources(VulkanInfo& vulkanInfo)
{
	CreateImage(vulkanInfo, vulkanInfo.swapChainExtent.width, vulkanInfo.swapChainExtent.height, vulkanInfo.depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanInfo.depthImage, vulkanInfo.depthImageMemory, 1, vulkanInfo.msaaSamples);

	CreateImageView(vulkanInfo.device, vulkanInfo.depthImageView, vulkanInfo.depthImage, vulkanInfo.depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);

	TransitionImageLayout(vulkanInfo, vulkanInfo.depthImage, vulkanInfo.depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1);
}

VkRenderPass CreateRenderPass(VkDevice device)
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = VK_FORMAT_B8G8R8A8_UNORM;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; // 采样数
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // clear
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // 存储下来
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VK_FORMAT_D32_SFLOAT;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//VkAttachmentReference colorAttachmentResolveRef = {};
	//colorAttachmentResolveRef.attachment = 2;
	//colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	subpass.pResolveAttachments = nullptr;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // 开始之前的pass
	dependency.dstSubpass = 0; // 第一个subpass

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // A pass layout 转换的阶段
	dependency.srcAccessMask = 0; // A pass layout 转换的阶段的操作之后就可以进行转换

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // B pass 等待执行的阶段
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; //B pass 等待执行的操作

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkRenderPass renderPass;
	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create render pass!");
	}

	return renderPass;
}

void CreateCommandPool(VulkanInfo& vulkanInfo)
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(vulkanInfo);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	if (vkCreateCommandPool(vulkanInfo.device, &poolInfo, nullptr, &vulkanInfo.commandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create command pool!");
	}
}

void CreateCommandBuffers(VulkanInfo& vulkanInfo)
{
	vulkanInfo.commandBuffers.resize(vulkanInfo.swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vulkanInfo.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)vulkanInfo.commandBuffers.size();

	if (vkAllocateCommandBuffers(vulkanInfo.device, &allocInfo, vulkanInfo.commandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate command buffers!");
	}

}

PipelineCreateInfo CreatePipelineCreateInfo()
{
	PipelineCreateInfo pipelineCreateInfo;

	pipelineCreateInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineCreateInfo.vertexInputInfo.vertexBindingDescriptionCount = 1;

	pipelineCreateInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineCreateInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineCreateInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

	pipelineCreateInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipelineCreateInfo.viewportState.viewportCount = 1;
	pipelineCreateInfo.viewportState.scissorCount = 1;

	pipelineCreateInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipelineCreateInfo.rasterizer.depthClampEnable = VK_FALSE;
	pipelineCreateInfo.rasterizer.polygonMode = VK_POLYGON_MODE_FILL; // VK_POLYGON_MODE_LINE
	//pipelineCreateInfo.rasterizer.polygonMode = VK_POLYGON_MODE_LINE; // VK_POLYGON_MODE_LINE

	pipelineCreateInfo.rasterizer.lineWidth = 1.0f;

	pipelineCreateInfo.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	pipelineCreateInfo.rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	pipelineCreateInfo.rasterizer.depthBiasEnable = VK_FALSE;
	pipelineCreateInfo.rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	pipelineCreateInfo.rasterizer.depthBiasClamp = 0.0f; // Optional
	pipelineCreateInfo.rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	pipelineCreateInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipelineCreateInfo.multisampling.sampleShadingEnable = VK_TRUE;
	// pipelineCreateInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineCreateInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineCreateInfo.multisampling.minSampleShading = 0.2f; // Optional
	pipelineCreateInfo.multisampling.pSampleMask = nullptr; // Optional
	pipelineCreateInfo.multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	pipelineCreateInfo.multisampling.alphaToOneEnable = VK_FALSE; // Optional

	pipelineCreateInfo.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipelineCreateInfo.colorBlending.logicOpEnable = VK_FALSE;
	pipelineCreateInfo.colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	pipelineCreateInfo.colorBlending.attachmentCount = 1;
	pipelineCreateInfo.colorBlending.blendConstants[0] = 0.0f; // Optional
	pipelineCreateInfo.colorBlending.blendConstants[1] = 0.0f; // Optional
	pipelineCreateInfo.colorBlending.blendConstants[2] = 0.0f; // Optional
	pipelineCreateInfo.colorBlending.blendConstants[3] = 0.0f; // Optional

	pipelineCreateInfo.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipelineCreateInfo.depthStencil.depthTestEnable = VK_TRUE;
	pipelineCreateInfo.depthStencil.depthWriteEnable = VK_TRUE;
	pipelineCreateInfo.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;

	return pipelineCreateInfo;
}

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}

VkShaderModule CreateShaderModule(VkDevice device, std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();

	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

std::array<VkPipelineShaderStageCreateInfo, 2> CreaterShader(VkDevice device, std::string vertexFile, std::string fragmentFile)
{
	auto vertShaderCode = readFile(vertexFile);
	auto fragShaderCode = readFile(fragmentFile);

	VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	return { vertShaderStageInfo, fragShaderStageInfo };
}

void CreateGraphicsPipeline(VulkanInfo& vulkanInfo)
{
	std::string vertexFileName = "D:/Games/URPWithVulkan/UniversalRenderPipeline/shaders/VertexShader.vert.spv";
	std::string fragmentFileName = "D:/Games/URPWithVulkan/UniversalRenderPipeline/shaders/PixelShader.frag.spv";

	auto shaderStages = CreaterShader(vulkanInfo.device, vertexFileName, fragmentFileName);

	PipelineCreateInfo info = CreatePipelineCreateInfo();

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	info.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	info.dynamicState.dynamicStateCount = 2;
	info.dynamicState.pDynamicStates = dynamicStates;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	info.colorBlending.pAttachments = &colorBlendAttachment;

	VkGraphicsPipelineCreateInfo pipelineInfo = {};

	info.Apply(pipelineInfo);

	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.renderPass = vulkanInfo.renderPass;


	// empty vertex input for quad
	VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo{};
	pipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	pipelineInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;

	pipelineInfo.stageCount = (uint32_t)shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = &vulkanInfo.descriptorSetLayout; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	if (vkCreatePipelineLayout(vulkanInfo.device, &pipelineLayoutInfo, nullptr, &vulkanInfo.pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	pipelineInfo.layout = vulkanInfo.pipelineLayout;
	if (vkCreateGraphicsPipelines(vulkanInfo.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vulkanInfo.pipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	vkDestroyShaderModule(vulkanInfo.device, shaderStages[0].module, nullptr);
	vkDestroyShaderModule(vulkanInfo.device, shaderStages[1].module, nullptr);
}

void CreateFrameBuffer(VulkanInfo& vulkanInfo)
{
	vulkanInfo.swapChainFramebuffers.resize(vulkanInfo.swapChainImageViews.size());
	for (size_t i = 0; i < vulkanInfo.swapChainImageViews.size(); i++)
	{
		std::vector<VkImageView> attachments;
		attachments = { vulkanInfo.swapChainImageViews[i], vulkanInfo.depthImageView };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = vulkanInfo.renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = vulkanInfo.swapChainExtent.width;
		framebufferInfo.height = vulkanInfo.swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(vulkanInfo.device, &framebufferInfo, nullptr, &vulkanInfo.swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void CreateBuffer(const VulkanInfo& vulkanInfo, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;

	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vulkanInfo.device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vulkanInfo.device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(vulkanInfo, memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(vulkanInfo.device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(vulkanInfo.device, buffer, bufferMemory, 0);
}

void CreateSemaphores(VulkanInfo& vulkanInfo)
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	if (vkCreateSemaphore(vulkanInfo.device, &semaphoreInfo, nullptr, &vulkanInfo.imageAvailableSemaphore) != VK_SUCCESS ||
		vkCreateSemaphore(vulkanInfo.device, &semaphoreInfo, nullptr, &vulkanInfo.renderFinishedSemaphore) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create semaphores!");
	}

}

GLFWwindow* InitWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

	return window;
}

VulkanInfo InitVulkan()
{
	VulkanInfo vulkanInfo = {};

	vulkanInfo.window = InitWindow();
	CreateVkInstance(vulkanInfo);

	if (glfwCreateWindowSurface(vulkanInfo.instance, vulkanInfo.window, nullptr, &vulkanInfo.surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}

	vulkanInfo.physicsDevice = pickPhysicalDevice(vulkanInfo.instance);

	CreateDevice(vulkanInfo);

	CreateCommandPool(vulkanInfo);

	CreateSwapChainAndImageView(vulkanInfo);
	CreateDepthResources(vulkanInfo);

	vulkanInfo.renderPass = CreateRenderPass(vulkanInfo.device);
	CreateGraphicsPipeline(vulkanInfo);
	CreateFrameBuffer(vulkanInfo);
	CreateCommandBuffers(vulkanInfo);

	CreateSemaphores(vulkanInfo);

	return vulkanInfo;
}

void Cleanup(VulkanInfo& vulkanInfo)
{
	vkDestroySemaphore(vulkanInfo.device, vulkanInfo.renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(vulkanInfo.device, vulkanInfo.imageAvailableSemaphore, nullptr);

	vkDestroyCommandPool(vulkanInfo.device, vulkanInfo.commandPool, nullptr);
	vkDestroyDescriptorSetLayout(vulkanInfo.device, vulkanInfo.descriptorSetLayout, nullptr);
	vkDestroyPipeline(vulkanInfo.device, vulkanInfo.pipeline, nullptr);
	vkDestroyPipelineLayout(vulkanInfo.device, vulkanInfo.pipelineLayout, nullptr);

	vkDestroyRenderPass(vulkanInfo.device, vulkanInfo.renderPass, nullptr);

	vkDestroyImageView(vulkanInfo.device, vulkanInfo.depthImageView, nullptr);
	vkDestroyImage(vulkanInfo.device, vulkanInfo.depthImage, nullptr);
	vkFreeMemory(vulkanInfo.device, vulkanInfo.depthImageMemory, nullptr);

	for (size_t i = 0; i < vulkanInfo.swapChainFramebuffers.size(); i++)
	{
		vkDestroyFramebuffer(vulkanInfo.device, vulkanInfo.swapChainFramebuffers[i], nullptr);
	}

	for (size_t i = 0; i < vulkanInfo.swapChainImageViews.size(); i++)
	{
		vkDestroyImageView(vulkanInfo.device, vulkanInfo.swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(vulkanInfo.device, vulkanInfo.swapChain, nullptr);
	vkDestroyDevice(vulkanInfo.device, nullptr);
	vkDestroySurfaceKHR(vulkanInfo.instance, vulkanInfo.surface, nullptr);
	DestroyDebugUtilsMessengerEXT(vulkanInfo.instance, vulkanInfo.debugMessenger, nullptr);
	vkDestroyInstance(vulkanInfo.instance, nullptr);
	glfwDestroyWindow(vulkanInfo.window);
}

void BuildCommandBuffers(VulkanInfo& vulkanInfo)
{
	for (size_t i = 0; i < vulkanInfo.commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr; // Optional

		vkBeginCommandBuffer(vulkanInfo.commandBuffers[i], &beginInfo);

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vulkanInfo.renderPass;
		renderPassInfo.framebuffer = vulkanInfo.swapChainFramebuffers[i];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vulkanInfo.swapChainExtent;

		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)vulkanInfo.swapChainExtent.width;
		viewport.height = (float)vulkanInfo.swapChainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor = {};
		scissor.offset = { 0, 0 };
		scissor.extent = vulkanInfo.swapChainExtent;

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(vulkanInfo.commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdSetViewport(vulkanInfo.commandBuffers[i], 0, 1, &viewport);
		vkCmdSetScissor(vulkanInfo.commandBuffers[i], 0, 1, &scissor);

		vkCmdBindPipeline(vulkanInfo.commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanInfo.pipeline);
		vkCmdDraw(vulkanInfo.commandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(vulkanInfo.commandBuffers[i]);

		if (vkEndCommandBuffer(vulkanInfo.commandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void DrawFrame(VulkanInfo& vulkanInfo)
{
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(vulkanInfo.device, vulkanInfo.swapChain, std::numeric_limits<uint64_t>::max(), vulkanInfo.imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);

	if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { vulkanInfo.imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &vulkanInfo.commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { vulkanInfo.renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	if (vkQueueSubmit(vulkanInfo.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { vulkanInfo.swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr; // Optional

	result = vkQueuePresentKHR(vulkanInfo.presentQueue, &presentInfo);

	 if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	vkQueueWaitIdle(vulkanInfo.presentQueue);

}

void MainLoop(VulkanInfo& vulkanInfo)
{
	while (!glfwWindowShouldClose(vulkanInfo.window)) 
	{
		glfwPollEvents();
		BuildCommandBuffers(vulkanInfo);
		DrawFrame(vulkanInfo);
	}

	vkDeviceWaitIdle(vulkanInfo.device);
}

int main()
{
	std::string command = "Shaders\\compile.bat";

	int result = std::system(command.c_str());

	VulkanInfo vulkanInfo = InitVulkan();

	MainLoop(vulkanInfo);

	Cleanup(vulkanInfo);

	getchar();
	return 0;
}