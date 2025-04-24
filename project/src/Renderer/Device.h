#pragma once
#include <optional>
#include <vulkan/vulkan.h>

#include "Instance.h"
#include "vk_mem_alloc.h"
#include "Window.h"

namespace gp2
{
	class Device
	{
	public:

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphicsFamily;
			std::optional<uint32_t> presentFamily;

			bool IsComplete()
			{
				return graphicsFamily.has_value() && presentFamily.has_value();
			}
		};

		struct SwapChainSupportDetails
		{
			VkSurfaceCapabilitiesKHR capabilities;
			std::vector<VkSurfaceFormatKHR> formats;
			std::vector<VkPresentModeKHR> presentModes;
		};


	public:
		Device(const Window* window);
		~Device();

		SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device) const;
		VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;
		uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;

		VkDevice GetLogicalDevice() const { return m_LogicalDevice; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }

		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }

		VmaAllocator GetAllocator() const { return m_Allocator; }

		static bool HasStencilComponent(VkFormat format);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
		bool IsDeviceSuitable(VkPhysicalDevice device) const;
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device) const;
		QueueFamilyIndices FindQueueFamilies() const;


	private:
		void PickPhysicalDevice();
		void CreateLogicalDevice();
		void SetupVMA();


		Instance m_Instance;
		
		VkPhysicalDevice m_PhysicalDevice{ VK_NULL_HANDLE };
		VkDevice m_LogicalDevice{};

		VkSurfaceKHR m_Surface{};

		VkQueue m_GraphicsQueue{};
		VkQueue m_PresentQueue{};

		VmaAllocator m_Allocator{};

		const std::vector<const char*> m_ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
		const std::vector<const char*> m_DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	};
}
