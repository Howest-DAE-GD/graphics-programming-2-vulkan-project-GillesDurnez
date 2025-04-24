#include "Window.h"

#include <stdexcept>

gp2::Window::Window(const std::string& title, uint32_t width, uint32_t height)
	: m_WindowTitle(title)
	, WIDTH(width)
	, HEIGHT(height)
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(m_pWindow, this);
    glfwSetFramebufferSizeCallback(m_pWindow, FramebufferResizeCallback);
}

void gp2::Window::CreateVkSurface(Instance& instance, VkSurfaceKHR* vkSurface) const
{
	if (glfwCreateWindowSurface( instance.GetInstance(), m_pWindow, nullptr, vkSurface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create window surface!");
	}
}

bool gp2::Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_pWindow);
}

void gp2::Window::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    auto app = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    app->m_FramebufferResized = true;
}

gp2::Window::~Window()
{
    glfwDestroyWindow(m_pWindow);
    glfwTerminate();
}
