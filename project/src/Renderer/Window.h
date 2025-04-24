#pragma once

#include <string>

#include "Instance.h"
#include "GLFW/glfw3.h"
#include "glm/vec2.hpp"

namespace gp2
{
	class Window
	{
	public:
		Window(const std::string& title = "Vulkan", uint32_t width = 800, uint32_t height = 600);
		~Window();

		Window(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&&) = delete;

		glm::i32vec2 GetWindowSize() const { return {WIDTH, HEIGHT}; }

		uint32_t GetWidth() const { return WIDTH; }
		uint32_t GetHeight() const { return HEIGHT; }

		GLFWwindow* GetWindow() const { return m_pWindow; }

		bool IsFrameBufferResized() const { return m_FramebufferResized; }
		void SetFrameBufferResized(bool state) { m_FramebufferResized = state; }

		void CreateVkSurface(Instance& instance, VkSurfaceKHR* vkSurface) const;

		bool ShouldClose() const;


	private:
		static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

		const uint32_t WIDTH{ 800 };
		const uint32_t HEIGHT{ 500 };

		bool m_FramebufferResized{ false };
		std::string m_WindowTitle{ "Vulkan" };

		GLFWwindow* m_pWindow{};
	};
}
