#pragma once
#include <vulkan/vulkan.h>

#include "Device.h"
#include "Resources/Image.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Buffer.h"

namespace gp2
{
	class SwapChain
	{
	public:
		SwapChain(Window* window, Device* device);
		~SwapChain();

		VkSwapchainKHR GetSwapChain() const { return m_SwapChain; }
		VkExtent2D GetSwapChainExtent() const { return m_SwapChainExtent; }
		VkFormat GetImageFormat() const { return m_SwapChainImageFormat; }
		std::vector<VkImageView> GetImageViews() const { return m_SwapChainImageViews; }

		void RecreateSwapChain();
		
		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

	private:
		void CreateSwapChain();
		void CreateImageViews();
		void CreateDepthResources();

		void CleanupSwapChain() const;

		static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		static VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;

		VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) const;
		VkFormat FindDepthFormat() const;
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;


		Window* m_pWindow{};
		Device* m_pDevice{};

		VkSwapchainKHR				m_SwapChain{};
		std::vector<VkImage>		m_SwapChainImages;
		VkFormat					m_SwapChainImageFormat{};
		VkExtent2D					m_SwapChainExtent{};
		std::vector<VkImageView>	m_SwapChainImageViews{};

	};
}
