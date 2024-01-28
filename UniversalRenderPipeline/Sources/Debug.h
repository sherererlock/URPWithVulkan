#pragma once
#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "vulkan/vulkan.h"

namespace debugutils
{
	void setup(VkInstance instance);
	void cmdBeginLabel(VkCommandBuffer cmdbuffer, std::string caption, glm::vec4 color);
	void cmdEndLabel(VkCommandBuffer cmdbuffer);
}

