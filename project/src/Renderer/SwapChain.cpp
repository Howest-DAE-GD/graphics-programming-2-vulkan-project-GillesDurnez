#include "SwapChain.h"

#include <algorithm>
#include <array>
#include <stdexcept>

#include "Buffer.h"

gp2::SwapChain::SwapChain(Window* window, Device* device, CommandPool* pCommandPool)
	: m_pWindow(window), m_pDevice(device), m_pCommandPool(pCommandPool)
{
    CreateSwapChain();
}

gp2::SwapChain::~SwapChain()
{

    CleanupSwapChain();
}

void gp2::SwapChain::CreateSwapChain()
{
    Device::SwapChainSupportDetails swapChainSupport = m_pDevice->QuerySwapChainSupport(m_pDevice->GetPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

    const uint32_t desiredImageCount = 2;
    uint32_t imageCount{};
    if (swapChainSupport.capabilities.maxImageCount > 0)
        imageCount = desiredImageCount;
    else
        imageCount = std::clamp(desiredImageCount, swapChainSupport.capabilities.minImageCount, swapChainSupport.capabilities.maxImageCount);

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_pDevice->GetSurface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    Device::QueueFamilyIndices indices = m_pDevice->FindQueueFamilies(m_pDevice->GetPhysicalDevice());
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(m_pDevice->GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create swap chain!");
    }

	std::vector<VkImage> swapChainImages;

    vkGetSwapchainImagesKHR(m_pDevice->GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
    m_SwapChainImages.clear();
	swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_pDevice->GetLogicalDevice(), m_SwapChain, &imageCount, swapChainImages.data());

	for (size_t i = 0; i < swapChainImages.size(); i++)
	{
		m_SwapChainImages.emplace_back(Image{ m_pDevice, m_pCommandPool, swapChainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT });
		m_pDevice->GetDebugger().SetDebugName(reinterpret_cast<uint64_t>(m_SwapChainImages[i].GetImage()), "SwapChain Image ", VK_OBJECT_TYPE_IMAGE);
    }

    m_SwapChainImageFormat = surfaceFormat.format;
    m_SwapChainExtent = extent;
}

VkSurfaceFormatKHR gp2::SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR gp2::SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D gp2::SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(m_pWindow->GetWindow(), &width, &height);

        VkExtent2D actualExtent =
        {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void gp2::SwapChain::CleanupSwapChain()
{
    vkDestroySwapchainKHR(m_pDevice->GetLogicalDevice(), m_SwapChain, nullptr);
}

void gp2::SwapChain::RecreateSwapChain()
{
    int width = m_pWindow->GetWidth();
    int height = m_pWindow->GetHeight();


    while (width == 0 || height == 0)
    {
        width = m_pWindow->GetWidth();
        height = m_pWindow->GetHeight();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(m_pDevice->GetLogicalDevice());

    CleanupSwapChain();

    CreateSwapChain();
}
