#include<stdexcept>

#include "ShWindow.h"
#include "InputController.h"

void ShWindow::framebufferResizeCallback(GLFWwindow* window, int w, int h)
{
	auto shWindow = reinterpret_cast<ShWindow*>(glfwGetWindowUserPointer(window));
	shWindow->framebufferResized = true;
	shWindow->width = w;
	shWindow->height = h;
}

void ShWindow::initWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);

	glfwSetWindowUserPointer(window, this);
	glfwSetWindowSizeCallback(window, Input::onWindowResized);
	glfwSetKeyCallback(window, Input::KeyCallback);
	glfwSetCursorPosCallback(window, Input::MouseCallback);
	glfwSetMouseButtonCallback(window, Input::MouseButtonCallback);
	glfwSetScrollCallback(window, Input::ScrollCallback);
}

ShWindow::ShWindow(int w, int h, std::string name)
	: width(w), height(h), windowName(name)
{
	initWindow();
}

ShWindow::~ShWindow()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

void ShWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface");
	}
}
