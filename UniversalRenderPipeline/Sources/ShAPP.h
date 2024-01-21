#pragma once
// std
#include <memory>
#include <vector>

#include "shdescriptors.h"
#include "shdevice.h"
#include "shgameobject.h"
#include "shrenderer.h"
#include "shwindow.h"
#include "UI/VulkanUIOverlay.h"

class Input;

class ShAPP
{
private:
	void loadGameObjects();

	ShWindow shWindow{ WIDTH, HEIGHT, "Vulkan Tutorial" };
	ShDevice shDevice{ shWindow };
	ShRenderer shRenderer{ shWindow, shDevice };

	// note: order of declarations matters
	std::unique_ptr<ShDescriptorPool> globalPool{};
	ShGameObject::Map gameObjects;
	vks::UIOverlay UIOverlay;
	float frameTimer;
	uint32_t lastFPS = 0;
public:
	static constexpr int WIDTH = 1920;
	static constexpr int HEIGHT = 1080;
	static constexpr int ShadowResolution = 2048;

	ShAPP();
	~ShAPP();

	ShAPP(const ShAPP&) = delete;
	ShAPP& operator=(const ShAPP&) = delete;

	void run();
	void nextFrame();
	void updateOverlay(Input& input);

	void drawUI(const VkCommandBuffer commandBuffer);
	void createUIOverlay();
	void OnUpdateUIOverlay();
	void buildCommandBuffer();

	bool lightUpdate = false;

};

