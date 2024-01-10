// std
#include <limits>

#include "InputController.h"

Input* Input::instance = nullptr;

void KeyboardMovementController::moveInPlaneXZ(
	GLFWwindow* window, float dt, ShGameObject& gameObject) {
	glm::vec3 rotate{ 0 };
	if (glfwGetKey(window, keys.lookRight) == GLFW_PRESS) rotate.y += 1.f;
	if (glfwGetKey(window, keys.lookLeft) == GLFW_PRESS) rotate.y -= 1.f;
	if (glfwGetKey(window, keys.lookUp) == GLFW_PRESS) rotate.x += 1.f;
	if (glfwGetKey(window, keys.lookDown) == GLFW_PRESS) rotate.x -= 1.f;

	if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
		gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
	}

	// limit pitch values between about +/- 85ish degrees
	gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
	gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

	float yaw = gameObject.transform.rotation.y;
	const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
	const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
	const glm::vec3 upDir{ 0.f, -1.f, 0.f };

	glm::vec3 moveDir{ 0.f };
	if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
	if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
	if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
	if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
	if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
	if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

	if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
		gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
	}
}

void Input::onWindowResized(GLFWwindow* window, int width, int height)
{
	if (width == 0 || height == 0) return;
}

void Input::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	Camera2& camera = instance->camera;
	if (key == GLFW_KEY_W)
	{
		if (action == GLFW_PRESS)
			camera.keys.up = true;
		if (action == GLFW_RELEASE)
			camera.keys.up = false;
	}

	if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
			camera.keys.down = true;
		if (action == GLFW_RELEASE)
			camera.keys.down = false;
	}

	if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS)
			camera.keys.left = true;
		if (action == GLFW_RELEASE)
			camera.keys.left = false;
	}

	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS)
			camera.keys.right = true;
		if (action == GLFW_RELEASE)
			camera.keys.right = false;
	}

	if (key == GLFW_KEY_Q)
	{
	}

	if (key == GLFW_KEY_T)
	{
	}

	if (key == GLFW_KEY_UP)
	{
	}

	if (key == GLFW_KEY_LEFT)
	{
	}
}

void Input::MouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	instance->MouseCallback(xpos, ypos);
}

void Input::MouseButtonCallback(GLFWwindow* window, int key, int action, int mods)
{
	Input& input = *instance;
	if (key == GLFW_MOUSE_BUTTON_LEFT)
	{
		if (action == GLFW_PRESS)
		{
			double xposition, yposition;
			glfwGetCursorPos(window, &xposition, &yposition);
			input.mousePos = glm::vec2((float)xposition, (float)yposition);
			input.mouseButtons.left = true;
		}
		else if (action == GLFW_RELEASE)
		{
			input.mouseButtons.left = false;
		}
	}

	if (key == GLFW_MOUSE_BUTTON_RIGHT)
	{
		if (action == GLFW_PRESS)
		{
			double xposition, yposition;
			glfwGetCursorPos(window, &xposition, &yposition);
			input.mousePos = glm::vec2((float)xposition, (float)yposition);
			input.mouseButtons.right = true;
		}
		else if (action == GLFW_RELEASE)
		{
			input.mouseButtons.right = false;
		}
	}

	if (key == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (action == GLFW_PRESS)
		{
			double xposition, yposition;
			glfwGetCursorPos(window, &xposition, &yposition);
			input.mousePos = glm::vec2((float)xposition, (float)yposition);

			input.mouseButtons.middle = true;
		}
		else if (action == GLFW_RELEASE)
		{
			input.mouseButtons.middle = false;
		}
	}
}

void Input::MouseCallback(double x, double y)
{
	int32_t dx = (int32_t)(mousePos.x - x);
	int32_t dy = (int32_t)(mousePos.y - y);

	if (mouseButtons.left) {
		camera.rotate(glm::vec3(dy * camera.rotationSpeed, -dx * camera.rotationSpeed, 0.0f));
	}
	if (mouseButtons.right) {
		camera.translate(glm::vec3(-0.0f, 0.0f, dy * .005f));
	}
	if (mouseButtons.middle) {
		camera.translate(glm::vec3(-dx * 0.005f, -dy * 0.005f, 0.0f));
	}
	mousePos = glm::vec2((float)x, (float)y);
}

void Input::ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	Camera2& camera = instance->camera;
	camera.translate(glm::vec3(0.0f, 0.0f, (float)yoffset * 0.005f));
}