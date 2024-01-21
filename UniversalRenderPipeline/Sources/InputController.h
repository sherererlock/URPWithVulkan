#pragma once

#include "shgameobject.h"
#include "shwindow.h"
#include "Camera.hpp"
#include "ShAPP.h"

class KeyboardMovementController {
public:
	struct KeyMappings {
		int moveLeft = GLFW_KEY_A;
		int moveRight = GLFW_KEY_D;
		int moveForward = GLFW_KEY_W;
		int moveBackward = GLFW_KEY_S;
		int moveUp = GLFW_KEY_E;
		int moveDown = GLFW_KEY_Q;
		int lookLeft = GLFW_KEY_LEFT;
		int lookRight = GLFW_KEY_RIGHT;
		int lookUp = GLFW_KEY_UP;
		int lookDown = GLFW_KEY_DOWN;
	};

	void moveInPlaneXZ(GLFWwindow* window, float dt, ShGameObject& gameObject);

	KeyMappings keys{};
	float moveSpeed{ 3.f };
	float lookSpeed{ 1.5f };
};

class Input
{
private:


	Camera2& camera;
	ShAPP& app;
	static Input* instance;
	float interval = 0.5f;
public:
	struct {
		bool left = false;
		bool right = false;
		bool middle = false;
	} mouseButtons;
	glm::vec2 mousePos;

	Input(Camera2& camera,ShAPP& app) : camera(camera), app(app) { instance = this; }

	void update(float deltaTime);

	static void onWindowResized(GLFWwindow* window, int width, int height);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseCallback(GLFWwindow* window, double xpos, double ypos);
	static void MouseButtonCallback(GLFWwindow* window, int key, int action, int mods);
	static void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

	void MouseCallback(double xpos, double ypos);
};
