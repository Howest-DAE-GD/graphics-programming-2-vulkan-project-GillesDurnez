#pragma once
#include <vulkan/vulkan.h>

#include "Device.h"
#include "Resources/Image.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Buffer.h"

namespace gp2
{
	class SwapChain
	{
	public:
		SwapChain(Window* window, Device* device, CommandPool* pCommandPool);
		~SwapChain();

		VkSwapchainKHR GetSwapChain() const { return m_SwapChain; }
		VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
		const VkFormat& GetImageFormat() const { return m_SwapChainImageFormat; }
		std::vector<Image>& GetImages() { return m_SwapChainImages; }
		
		void RecreateSwapChain();
		
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	private:
		void CreateSwapChain();

		void CleanupSwapChain();

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

		Window* m_pWindow{};
		Device* m_pDevice{};
		CommandPool* m_pCommandPool{};

		VkSwapchainKHR				m_SwapChain{};
		std::vector<Image>			m_SwapChainImages{};
		VkFormat					m_SwapChainImageFormat{};
		VkExtent2D					m_SwapChainExtent{};

	};
}
