#pragma once
#include <vector>
#include <vulkan/vulkan.h>

namespace gp2
{
	class Instance
	{
	public:
		Instance();
		~Instance();

		Instance(const Instance&) = delete;
		Instance(Instance&&) = delete;
		Instance& operator=(const Instance&) = delete;
		Instance& operator=(Instance&&) = delete;

		VkInstance GetInstance() const { return m_Instance; }
		bool IsValidationLayersEnabled() const { return m_EnableValidationLayers; }

	private:
		void CreateInstance();
		void SetupDebugMessenger();

		bool CheckValidationLayerSupport() const;
		std::vector<const char*> GetRequiredExtensions() const;

		static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
		void DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) const;

		VkResult CreateDebugUtilsMessengerEXT(
			const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator,
			VkDebugUtilsMessengerEXT* pDebugMessenger) const;

		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData);
		
		VkInstance m_Instance{};
		VkDebugUtilsMessengerEXT m_vkDebugMessenger{};

		const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
		const bool m_EnableValidationLayers = false;
#else
		const bool m_EnableValidationLayers = true;
#endif

	};
}
