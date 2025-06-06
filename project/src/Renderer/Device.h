#pragma once
#include <iostream>
#include <optional>
#include <vulkan/vulkan.h>

#include "Instance.h"
#include "vk_mem_alloc.h"
#include "Window.h"
#include "glm/vec4.hpp"

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

		class DeviceDebugger
		{
		public:
			DeviceDebugger(VkDevice pDevice) : m_pDevice(pDevice) {}

			void SetDebugName(uint64_t object ,const std::string& name, VkObjectType objectType) const
			{
#ifdef _DEBUG
				VkDebugUtilsObjectNameInfoEXT nameInfo{};
				nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
				nameInfo.objectType = objectType;
				nameInfo.objectHandle = object;
				nameInfo.pObjectName = name.c_str();
				VkResult result = vkSetDebugUtilsObjectNameEXT(m_pDevice, &nameInfo);
				if (result != VK_SUCCESS) 
				{
					// Handle error (rare), e.g. log or assert
					std::cout << "Failed to set debug name for object: " << name << std::endl;
				}
#endif
			}

			void BeginLabel(VkCommandBuffer commandBuffer, const std::string& labelName, const glm::vec4& color) const
			{
#ifdef _DEBUG

				VkDebugUtilsLabelEXT labelInfo{};
				labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
				labelInfo.pLabelName = labelName.c_str();
				labelInfo.color[0] = color.r;
				labelInfo.color[1] = color.g;
				labelInfo.color[2] = color.b;
				labelInfo.color[3] = color.a;

				vkCmdBeginDebugUtilsLabelEXT(commandBuffer, &labelInfo);
#endif
			}

			void EndLabel(VkCommandBuffer commandBuffer) const
			{
#ifdef _DEBUG

				vkCmdEndDebugUtilsLabelEXT(commandBuffer);
#endif
			}

			void InsertLabel(VkCommandBuffer commandBuffer, const std::string& labelName, const glm::vec4& color) const
			{
#ifdef _DEBUG

				VkDebugUtilsLabelEXT labelInfo{};
				labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
				labelInfo.pLabelName = labelName.c_str();
				labelInfo.color[0] = color.r;
				labelInfo.color[1] = color.g;
				labelInfo.color[2] = color.b;
				labelInfo.color[3] = color.a;

				vkCmdInsertDebugUtilsLabelEXT(commandBuffer, &labelInfo);
#endif
			}


		private:
			VkDevice m_pDevice;
			PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>( vkGetDeviceProcAddr(m_pDevice, "vkSetDebugUtilsObjectNameEXT")	);
			PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetDeviceProcAddr(m_pDevice, "vkCmdBeginDebugUtilsLabelEXT"));
			PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetDeviceProcAddr(m_pDevice, "vkCmdEndDebugUtilsLabelEXT"));
			PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(vkGetDeviceProcAddr(m_pDevice, "vkCmdInsertDebugUtilsLabelEXT"));
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
		VkFormat FindDepthFormat() const;


		VkDevice GetLogicalDevice() const { return m_LogicalDevice; }
		VkPhysicalDevice GetPhysicalDevice() const { return m_PhysicalDevice; }
		VkSurfaceKHR GetSurface() const { return m_Surface; }

		VkQueue GetGraphicsQueue() const { return m_GraphicsQueue; }
		VkQueue GetPresentQueue() const { return m_PresentQueue; }

		const DeviceDebugger& GetDebugger() const { return *m_pDebugger; }

		VmaAllocator GetAllocator() const { return m_Allocator; }

		static bool HasStencilComponent(VkFormat format);
		bool CheckDeviceExtensionSupport(VkPhysicalDevice device) const;
		int RateDeviceSuitability(VkPhysicalDevice device) const;

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

		DeviceDebugger* m_pDebugger{};

	};
}
