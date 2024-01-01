#pragma once

#include<string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class ShWindow
{
private:
	GLFWwindow* window;
	int width; 
	int height;
	std::string windowName;
	bool framebufferResized = false;

	static void framebufferResizeCallback(GLFWwindow* window, int w, int h);
	void initWindow();
public :
	ShWindow(int w, int h, std::string name);
	~ShWindow();

	ShWindow(const ShWindow&) = delete;
	ShWindow& operator=(const ShWindow&) = delete;

	bool shouldClose() { return glfwWindowShouldClose(window); }
	VkExtent2D getExtent() { return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) }; }
	bool wasWindowResized() { return framebufferResized; }
	void resetWindowResizedFlag() { framebufferResized = false; }
	GLFWwindow* getGLFWwindow() const { return window; }

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);
};

