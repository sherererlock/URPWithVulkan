#pragma once
#include <fstream>
#include <stdlib.h>
#include <string>
#include <vector>

#include "vulkan/vulkan.h"

#include <ktx.h>
#include <ktxvulkan.h>

#include "ShDevice.h"
#include "Tools.h"

class ShTexture
{
protected:
	ShDevice& device;
	uint32_t width;
	uint32_t height;
	uint32_t mipLevels;
	uint32_t layerCount;
	VkImage image;
	VkImageView imageView;
	VkDeviceMemory deviceMemory;
	VkSampler sampler;
	VkImageLayout imageLayout;

public:
	
	ShTexture(ShDevice& device);
	~ShTexture();
	ShTexture(const ShTexture&) = delete;
	ShTexture& operator=(const ShTexture&) = delete;

	VkDescriptorImageInfo descriptorImageInfo();
	ktxResult loadKTXFile(std::string filename, ktxTexture** target);
};

class ShTexture2D : public ShTexture
{
public:
	void loadFromFile(
		std::string        filename,
		VkFormat           format,
		VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
		VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		bool               forceLinear = false);
	void fromBuffer(
		void* buffer,
		VkDeviceSize       bufferSize,
		VkFormat           format,
		uint32_t           texWidth,
		uint32_t           texHeight,
		VkFilter           filter = VK_FILTER_LINEAR,
		VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
		VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	ShTexture2D(ShDevice& device) : ShTexture(device) {}
};

class ShTextureCubeMap : public ShTexture
{
public:
	void loadFromFile(
		std::string        filename,
		VkFormat           format,
		VkImageUsageFlags  imageUsageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
		VkImageLayout      imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
};