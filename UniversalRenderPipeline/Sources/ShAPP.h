#pragma once
// std
#include <memory>
#include <vector>

#include "shdescriptors.h"
#include "shdevice.h"
#include "shgameobject.h"
#include "shrenderer.h"
#include "shwindow.h"

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

public:
	static constexpr int WIDTH = 1920;
	static constexpr int HEIGHT = 1080;
	static constexpr int ShadowResolution = 2048;

	ShAPP();
	~ShAPP();

	ShAPP(const ShAPP&) = delete;
	ShAPP& operator=(const ShAPP&) = delete;

	void run();

	bool lightUpdate = false;

};

