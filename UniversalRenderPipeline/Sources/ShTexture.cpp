
#include <cassert>
#include <iostream>

#include "ShTexture.h"
#include "VulkanInitializers.hpp"

ShTexture::ShTexture(ShDevice& device)
	: device(device)
{
}

ShTexture::~ShTexture()
{
	vkDestroyImageView(device.device(), imageView, nullptr);
	vkDestroyImage(device.device(), image, nullptr);
	if (sampler)
	{
		vkDestroySampler(device.device(), sampler, nullptr);
	}
	vkFreeMemory(device.device(), deviceMemory, nullptr);
}

VkDescriptorImageInfo ShTexture::descriptorImageInfo()
{
	return VkDescriptorImageInfo { sampler, imageView, imageLayout};
}

ktxResult ShTexture::loadKTXFile(std::string filename, ktxTexture** target)
{
	ktxResult result = KTX_SUCCESS;

	if (!Tools::fileExists(filename)) {
		Tools::exitFatal("Could not load texture from " + filename + "\n\nMake sure the assets submodule has been checked out and is up-to-date.", -1);
	}
	result = ktxTexture_CreateFromNamedFile(filename.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, target);

	return result;
}

void ShTexture2D::loadFromFile(std::string filename, VkFormat format, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout, bool forceLinear)
{
	ktxTexture* ktxTexture;
	ktxResult result = loadKTXFile(filename, &ktxTexture);
	assert(result == KTX_SUCCESS);

	width = ktxTexture->baseWidth;
	height = ktxTexture->baseHeight;
	mipLevels = ktxTexture->numLevels;

	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

	// Get device properties for the requested texture format
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(device.physicalDevice(), format, &formatProperties);

	VkMemoryAllocateInfo memAllocInfo = Initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;

	VkCommandBuffer copyCmd = device.beginSingleTimeCommands();

	// Create a host-visible staging buffer that contains the raw image data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = Initializers::bufferCreateInfo();
	bufferCreateInfo.size = ktxTextureSize;
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device.device(), &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	vkGetBufferMemoryRequirements(device.device(), stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device.device(), stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t* data;
	VK_CHECK_RESULT(vkMapMemory(device.device(), stagingMemory, 0, memReqs.size, 0, (void**)&data));
	memcpy(data, ktxTextureData, ktxTextureSize);
	vkUnmapMemory(device.device(), stagingMemory);

	// Setup buffer copy regions for each mip level
	std::vector<VkBufferImageCopy> bufferCopyRegions;

	for (uint32_t i = 0; i < mipLevels; i++)
	{
		ktx_size_t offset;
		KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, i, 0, 0, &offset);
		assert(result == KTX_SUCCESS);

		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = std::max(1u, ktxTexture->baseWidth >> i);
		bufferCopyRegion.imageExtent.height = std::max(1u, ktxTexture->baseHeight >> i);
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);
	}

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = Initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;

	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.usage = imageUsageFlags;
	// Ensure that the TRANSFER_DST bit is set for staging
	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	VK_CHECK_RESULT(vkCreateImage(device.device(), &imageCreateInfo, nullptr, &image));

	vkGetImageMemoryRequirements(device.device(), image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;

	memAllocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device.device(), image, deviceMemory, 0));

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = mipLevels;
	subresourceRange.layerCount = 1;

	// Image barrier for optimal image (target)
	// Optimal image will be used as destination for the copy
	Tools::setImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// Copy mip levels from staging buffer
	vkCmdCopyBufferToImage(
		copyCmd,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data()
	);

	// Change texture image layout to shader read after all mip levels have been copied
	this->imageLayout = imageLayout;
	Tools::setImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		imageLayout,
		subresourceRange);

	device.endSingleTimeCommands(copyCmd);

	// Clean up staging resources
	vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
	vkFreeMemory(device.device(), stagingMemory, nullptr);

	ktxTexture_Destroy(ktxTexture);

	// Create a default sampler
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	// Max level-of-detail should match mip level count
	samplerCreateInfo.maxLod = (float)mipLevels;
	// Only enable anisotropic filtering if enabled on the device
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.anisotropyEnable = true;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device.device(), &samplerCreateInfo, nullptr, &sampler));

	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	viewCreateInfo.subresourceRange.levelCount = mipLevels;
	viewCreateInfo.image = image;
	VK_CHECK_RESULT(vkCreateImageView(device.device(), &viewCreateInfo, nullptr, &imageView));
}

void ShTexture2D::fromBuffer(void* buffer, VkDeviceSize bufferSize, VkFormat format, uint32_t texWidth, uint32_t texHeight, VkFilter filter, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
{
	assert(buffer);

	width = texWidth;
	height = texHeight;
	mipLevels = 1;

	VkMemoryAllocateInfo memAllocInfo = Initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;

	// Use a separate command buffer for texture loading
	VkCommandBuffer copyCmd = device.beginSingleTimeCommands();

	// Create a host-visible staging buffer that contains the raw image data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = Initializers::bufferCreateInfo();
	bufferCreateInfo.size = bufferSize;
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device.device(), &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	vkGetBufferMemoryRequirements(device.device(), stagingBuffer, &memReqs);


	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device.device(), stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t* data;
	VK_CHECK_RESULT(vkMapMemory(device.device(), stagingMemory, 0, memReqs.size, 0, (void**)&data));
	memcpy(data, buffer, bufferSize);
	vkUnmapMemory(device.device(), stagingMemory);

	VkBufferImageCopy bufferCopyRegion = {};
	bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	bufferCopyRegion.imageSubresource.mipLevel = 0;
	bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
	bufferCopyRegion.imageSubresource.layerCount = 1;
	bufferCopyRegion.imageExtent.width = width;
	bufferCopyRegion.imageExtent.height = height;
	bufferCopyRegion.imageExtent.depth = 1;
	bufferCopyRegion.bufferOffset = 0;

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = Initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.usage = imageUsageFlags;
	// Ensure that the TRANSFER_DST bit is set for staging
	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	VK_CHECK_RESULT(vkCreateImage(device.device(), &imageCreateInfo, nullptr, &image));

	vkGetImageMemoryRequirements(device.device(), image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;

	memAllocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device.device(), image, deviceMemory, 0));

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = mipLevels;
	subresourceRange.layerCount = 1;

	// Image barrier for optimal image (target)
	// Optimal image will be used as destination for the copy
	Tools::setImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// Copy mip levels from staging buffer
	vkCmdCopyBufferToImage(
		copyCmd,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&bufferCopyRegion
	);

	// Change texture image layout to shader read after all mip levels have been copied
	this->imageLayout = imageLayout;
	Tools::setImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		imageLayout,
		subresourceRange);

	device.endSingleTimeCommands(copyCmd);

	// Clean up staging resources
	vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
	vkFreeMemory(device.device(), stagingMemory, nullptr);


	// Create sampler
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = filter;
	samplerCreateInfo.minFilter = filter;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	VK_CHECK_RESULT(vkCreateSampler(device.device(), &samplerCreateInfo, nullptr, &sampler));

	// Create image view
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.pNext = NULL;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = format;
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.image = image;
	VK_CHECK_RESULT(vkCreateImageView(device.device(), &viewCreateInfo, nullptr, &imageView));

}

void ShTextureCubeMap::loadFromFile(std::string filename, VkFormat format, VkImageUsageFlags imageUsageFlags, VkImageLayout imageLayout)
{
	ktxTexture* ktxTexture;
	ktxResult result = loadKTXFile(filename, &ktxTexture);

	assert(result == KTX_SUCCESS);

	width = ktxTexture->baseWidth;
	height = ktxTexture->baseHeight;
	mipLevels = ktxTexture->numLevels;

	ktx_uint8_t* ktxTextureData = ktxTexture_GetData(ktxTexture);
	ktx_size_t ktxTextureSize = ktxTexture_GetSize(ktxTexture);

	VkMemoryAllocateInfo memAllocInfo = Initializers::memoryAllocateInfo();
	VkMemoryRequirements memReqs;

	// Create a host-visible staging buffer that contains the raw image data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = Initializers::bufferCreateInfo();
	bufferCreateInfo.size = ktxTextureSize;
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(device.device(), &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	vkGetBufferMemoryRequirements(device.device(), stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(device.device(), stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t* data;
	VK_CHECK_RESULT(vkMapMemory(device.device(), stagingMemory, 0, memReqs.size, 0, (void**)&data));
	memcpy(data, ktxTextureData, ktxTextureSize);
	vkUnmapMemory(device.device(), stagingMemory);

	// Setup buffer copy regions for each face including all of its mip levels
	std::vector<VkBufferImageCopy> bufferCopyRegions;

	for (uint32_t face = 0; face < 6; face++)
	{
		for (uint32_t level = 0; level < mipLevels; level++)
		{
			ktx_size_t offset;
			KTX_error_code result = ktxTexture_GetImageOffset(ktxTexture, level, 0, face, &offset);
			assert(result == KTX_SUCCESS);

			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = level;
			bufferCopyRegion.imageSubresource.baseArrayLayer = face;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = ktxTexture->baseWidth >> level;
			bufferCopyRegion.imageExtent.height = ktxTexture->baseHeight >> level;
			bufferCopyRegion.imageExtent.depth = 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions.push_back(bufferCopyRegion);
		}
	}

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = Initializers::imageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = mipLevels;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.usage = imageUsageFlags;
	// Ensure that the TRANSFER_DST bit is set for staging
	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	// Cube faces count as array layers in Vulkan
	imageCreateInfo.arrayLayers = 6;
	// This flag is required for cube map images
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

	VK_CHECK_RESULT(vkCreateImage(device.device(), &imageCreateInfo, nullptr, &image));

	vkGetImageMemoryRequirements(device.device(), image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_CHECK_RESULT(vkAllocateMemory(device.device(), &memAllocInfo, nullptr, &deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(device.device(), image, deviceMemory, 0));

	VkCommandBuffer copyCmd = device.beginSingleTimeCommands();

	// Image barrier for optimal image (target)
	// Set initial layout for all array layers (faces) of the optimal (target) tiled texture
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = mipLevels;
	subresourceRange.layerCount = 6;

	Tools::setImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// Copy the cube map faces from the staging buffer to the optimal tiled image
	vkCmdCopyBufferToImage(
		copyCmd,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data());

	// Change texture image layout to shader read after all faces have been copied
	this->imageLayout = imageLayout;
	Tools::setImageLayout(
		copyCmd,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		imageLayout,
		subresourceRange);

	device.endSingleTimeCommands(copyCmd);

	// Create sampler
	VkSamplerCreateInfo samplerCreateInfo = Initializers::samplerCreateInfo();
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeU;
	samplerCreateInfo.addressModeW = samplerCreateInfo.addressModeU;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.maxAnisotropy = 1.0f;
	samplerCreateInfo.anisotropyEnable = true;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = (float)mipLevels;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(device.device(), &samplerCreateInfo, nullptr, &sampler));

	// Create image view
	VkImageViewCreateInfo viewCreateInfo = Initializers::imageViewCreateInfo();
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	viewCreateInfo.format = format;
	viewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	viewCreateInfo.subresourceRange.layerCount = 6;
	viewCreateInfo.subresourceRange.levelCount = mipLevels;
	viewCreateInfo.image = image;
	VK_CHECK_RESULT(vkCreateImageView(device.device(), &viewCreateInfo, nullptr, &imageView));

	// Clean up staging resources
	ktxTexture_Destroy(ktxTexture);
	vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
	vkFreeMemory(device.device(), stagingMemory, nullptr);
}
