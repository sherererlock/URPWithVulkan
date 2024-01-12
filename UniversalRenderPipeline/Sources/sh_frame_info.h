#pragma once

#include "camera.hpp"
#include "ShGameObject.h"

// lib
#include <vulkan/vulkan.h>

#define MAX_LIGHTS 10

struct PointLight {
	glm::vec4 position{};  // ignore w
	glm::vec4 color{};     // w is intensity
};

struct GlobalUbo {
	glm::mat4 projection{ 1.f };
	glm::mat4 view{ 1.f };
	glm::vec4 ambientLightColor{ 1.f, 1.f, 1.f, .02f };  // w is intensity
	glm::vec4 viewPos;
	glm::vec4 camereInfo;
	glm::vec4 size;
	PointLight pointLights[MAX_LIGHTS];
	int numLights;
};

struct FrameInfo {
	int frameIndex;
	float frameTime;
	bool updateLight;
	VkCommandBuffer commandBuffer;
	Camera2& camera;
	VkDescriptorSet globalDescriptorSet;
	ShGameObject::Map& gameObjects;
};