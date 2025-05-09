#include "Instance.h"

#include <iostream>
#include <stdexcept>

#include "GLFW/glfw3.h"

gp2::Instance::Instance()
{
    CreateInstance();
    SetupDebugMessenger();
}

gp2::Instance::~Instance()
{
    if (m_EnableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_vkDebugMessenger, nullptr);
    }

    vkDestroyInstance(m_Instance, nullptr);
}

void gp2::Instance::CreateInstance()
{
#ifndef NDEBUG
    if (m_EnableValidationLayers && !CheckValidationLayerSupport())
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
    appInfo.apiVersion = VK_API_VERSION_1_3;

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
    if (m_EnableValidationLayers)
    {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (m_EnableValidationLayers)
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


    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create a vk instance!");
    }
}

void gp2::Instance::SetupDebugMessenger()
{
    if (!m_EnableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    PopulateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr, &m_vkDebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

bool gp2::Instance::CheckValidationLayerSupport() const
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : validationLayers) 
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) 
        {
            if (strcmp(layerName, layerProperties.layerName) == 0) 
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) 
        {
            return false;
        }
    }

    return true;
}

std::vector<const char*> gp2::Instance::GetRequiredExtensions() const
{
    uint32_t glfwExtensionCount = 0;

    // TODO: Check if you can extract this to the window class so knowledge of glfw is not needed here
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (m_EnableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void gp2::Instance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

void gp2::Instance::DestroyDebugUtilsMessengerEXT(VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) const
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(m_Instance, debugMessenger, pAllocator);
    }
}

VkResult gp2::Instance::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) const
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_Instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(m_Instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VkBool32 gp2::Instance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                      VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                      void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        // Message is important enough to show
        std::cerr << "+-----------------------------------------+\n" << "Validation layer: " << pCallbackData->pMessage << "\n\n";
    }

    return VK_FALSE;
}

