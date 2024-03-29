/*
* UI overlay class using ImGui
*
* Copyright (C) 2017 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <vector>
#include <sstream>
#include <iomanip>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vulkan/vulkan.h>
#include "ShDevice.h"
#include "ShBuffer.h"
#include "ShPipeline.h"

#include "imgui/imgui.h"

#if defined(__ANDROID__)
#include "VulkanAndroid.h"
#endif

namespace vks 
{
	class UIOverlay 
	{
	public:
		ShDevice *device;
		VkQueue queue;

		VkSampleCountFlagBits rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		uint32_t subpass = 0;

		std::unique_ptr<ShBuffer> vertexBuffer;
		std::unique_ptr<ShBuffer> indexBuffer;
		int32_t vertexCount = 0;
		int32_t indexCount = 0;

		std::vector<VkPipelineShaderStageCreateInfo> shaders;

		VkDescriptorPool descriptorPool;
		VkDescriptorSetLayout descriptorSetLayout;
		VkDescriptorSet descriptorSet;
		VkPipelineLayout pipelineLayout;
		VkPipeline pipeline;

		VkDeviceMemory fontMemory = VK_NULL_HANDLE;
		VkImage fontImage = VK_NULL_HANDLE;
		VkImageView fontView = VK_NULL_HANDLE;
		VkSampler sampler;

		struct PushConstBlock {
			glm::vec2 scale;
			glm::vec2 translate;
		} pushConstBlock;

		bool visible = true;
		bool updated = false;
		float scale = 1.0f;

		UIOverlay();
		~UIOverlay();

		void preparePipeline(const VkPipelineCache pipelineCache, const VkRenderPass renderPass, const VkFormat colorFormat, const VkFormat depthFormat);
		void prepareResources();

		bool update();
		void draw(const VkCommandBuffer commandBuffer);
		void resize(uint32_t width, uint32_t height);

		void freeResources();

		bool header(const char* caption);
		bool checkBox(const char* caption, bool* value);
		bool checkBox(const char* caption, int32_t* value);
		bool radioButton(const char* caption, bool value);
		bool inputFloat(const char* caption, float* value, float step, uint32_t precision);
		bool sliderFloat(const char* caption, float* value, float min, float max);
		bool sliderInt(const char* caption, int32_t* value, int32_t min, int32_t max);
		bool comboBox(const char* caption, int32_t* itemindex, std::vector<std::string> items);
		bool button(const char* caption);
		bool colorPicker(const char* caption, float* color);
		void text(const char* formatstr, ...);
	};
}
