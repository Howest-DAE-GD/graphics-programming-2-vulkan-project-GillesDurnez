#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <optional>

#include "GLFW/glfw3.h"

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;

    bool IsComplete()
	{
        return graphicsFamily.has_value();
    }
};

class HelloTriangleApplication
{
public:
    void Run()
	{
        InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:
    void SetupDebugMessenger()
	{
        if constexpr (!enableValidationLayers) return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(m_vkInstance, &createInfo, nullptr, &m_vkDebugMessenger) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance, 
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
        const VkAllocationCallbacks* pAllocator, 
        VkDebugUtilsMessengerEXT* pDebugMessenger)
	{
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else 
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
	{
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            func(instance, debugMessenger, pAllocator);
        }
    }
    void CreateInstance()
    {
#ifndef NDEBUG
        if (enableValidationLayers && !CheckValidationLayerSupport()) 
        {
            throw std::runtime_error("validation layers requested, but not available!");
        }
#endif

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        uint32_t glfwExtensionCount = 0;

        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();
        /*const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);*/

        //createInfo.enabledExtensionCount = glfwExtensionCount;
        //createInfo.ppEnabledExtensionNames = glfwExtensions;

        // For SDL
        // SDL_bool SDL_Vulkan_GetInstanceExtensions(SDL_Window *window, unsigned int* pCount, const char** pNames);
        if (enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else 
        {
            createInfo.enabledLayerCount = 0;
        }

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if constexpr (enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else 
        {
            createInfo.enabledLayerCount = 0;

            createInfo.pNext = nullptr;
        }


        VkResult result = vkCreateInstance(&createInfo, nullptr, &m_vkInstance);
        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create a vk instance!");
        }
    }

    void CleanupVulkan()
    {
        vkDestroyDevice(m_Device, nullptr);

        if (enableValidationLayers) 
        {
            DestroyDebugUtilsMessengerEXT(m_vkInstance, m_vkDebugMessenger, nullptr);
        }

        vkDestroyInstance(m_vkInstance, nullptr);
    }
	bool CheckValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (strcmp(layerName, layerProperties.layerName) == 0) {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> GetRequiredExtensions()
	{
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) 
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
	{
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            // Message is important enough to show
			std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    void PickPhysicalDevice()
	{
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, nullptr);

        if (deviceCount == 0)
        {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_vkInstance, &deviceCount, devices.data());

        for (const auto& device : devices) 
        {
            if (IsDeviceSuitable(device)) 
            {
                m_vkPhysicalDevice = device;
                break;
            }
        }

        if (m_vkPhysicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    bool IsDeviceSuitable(VkPhysicalDevice device)
	{
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        return true;
    }

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
	{
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) 
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
            {
                indices.graphicsFamily = i;
            }

            if (indices.IsComplete()) {
                break;
            }

            i++;
        }

        return indices;
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
	{
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    void CreateLogicalDevice()
	{
        QueueFamilyIndices indices = FindQueueFamilies(m_vkPhysicalDevice);

        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        VkPhysicalDeviceFeatures deviceFeatures{ VK_FALSE };
        VkDeviceCreateInfo createInfo{};

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueCreateInfo;
        createInfo.queueCreateInfoCount = 1;
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = 0;

        if (enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else 
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_vkPhysicalDevice, &createInfo, nullptr, &m_Device) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(m_Device, indices.graphicsFamily.value(), 0, &m_GraphicsQueue);
    }

    void InitWindow()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    	m_pWindow = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void InitVulkan()
	{
        CreateInstance();
        SetupDebugMessenger();
        PickPhysicalDevice();
        CreateLogicalDevice();
    }

    void MainLoop()
	{
        while(!glfwWindowShouldClose(m_pWindow)) 
        {
            glfwPollEvents();
        }
    }

    void Cleanup()
	{
        CleanupVulkan();

        glfwDestroyWindow(m_pWindow);

        glfwTerminate();
    }


    GLFWwindow* m_pWindow;
    VkInstance m_vkInstance;
    VkDebugUtilsMessengerEXT m_vkDebugMessenger;
    VkPhysicalDevice m_vkPhysicalDevice{ VK_NULL_HANDLE };
    VkDevice m_Device;
    VkQueue m_GraphicsQueue;
};

int main(int argc, char* argv[])
{
    HelloTriangleApplication app;

    try 
    {
        app.Run();
    }
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

 