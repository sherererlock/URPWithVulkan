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
#include "RenderPass/ShadowPass.h"
#include "RenderSystem/ShadowRenderSystem.h"
#include "RenderPass/BasePass.h"
#include "RenderPass/LightingPass.h"
#include "RenderPass/BlitPass.h"
#include "RenderSystem/BlitRenderSystem.h"
#include "RenderSystem/SimpleRenderSystem.h"
#include "RenderSystem/GltfRenderSystem.h"
#include "RenderSystem/PointLight.h"

class Input;

class ShAPP
{
private:
	void loadGameObjects();

	ShWindow shWindow{ WIDTH, HEIGHT, "Vulkan Tutorial" };
	ShDevice shDevice{ shWindow };
	ShRenderer shRenderer{ shWindow, shDevice };

	std::unique_ptr<ShadowPass> shadowPass;
	std::unique_ptr<ShadowRenderSystem> shadowRenderSystem;

	std::unique_ptr<BasePass> basePass;
	std::unique_ptr<GltfRenderSystem> baseRenderSystem;
	std::unique_ptr<LightingPass> lightPass;
	std::unique_ptr<BlitRenderSystem> lightingRenderSystem;

	std::unique_ptr<PointLightSystem> pointLightSystem;
	std::unique_ptr<BlitRenderSystem> blitRenderSystem;

	std::vector<VkDescriptorSet> blitDescriptorSets;
	std::vector<VkDescriptorSet> lightDescriptorSets;
	std::vector<VkDescriptorSet> imageDescriptorSets;

	// note: order of declarations matters
	std::unique_ptr<ShDescriptorPool> globalPool{};
	ShGameObject::Map gameObjects;
	vks::UIOverlay UIOverlay;
	float frameTimer;
	uint32_t lastFPS = 0;
public:
	static constexpr int WIDTH = 720;
	static constexpr int HEIGHT = 576;
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
	void buildCommandBuffer(uint32_t frameIndex, FrameInfo& frameInfo);

	bool lightUpdate = false;

};

