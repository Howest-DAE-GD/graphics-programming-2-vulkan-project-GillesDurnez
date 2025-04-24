#include <algorithm>
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <optional>
#include <set>
#include <array>
#include <chrono>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <unordered_map>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "Renderer/Window.h"
#include "Renderer/Device.h"
#include "Renderer/SwapChain.h"
#include "Renderer/RenderPass.h"
#include "Renderer/Pipeline.h"
#include "Renderer/Resources/Model.h"


//const uint32_t WIDTH = 800;
//const uint32_t HEIGHT = 600;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

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

//struct Vertex
//{
//    glm::vec3 pos;
//    glm::vec3 color;
//    glm::vec2 texCoord;
//
//    static VkVertexInputBindingDescription getBindingDescription()
//	{
//        VkVertexInputBindingDescription bindingDescription{};
//        bindingDescription.binding = 0;
//        bindingDescription.stride = sizeof(Vertex);
//        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//        return bindingDescription;
//    }
//
//    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
//	{
//        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
//
//        attributeDescriptions[0].binding = 0;
//        attributeDescriptions[0].location = 0;
//        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[0].offset = offsetof(Vertex, pos);
//
//
//        attributeDescriptions[1].binding = 0;
//        attributeDescriptions[1].location = 1;
//        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[1].offset = offsetof(Vertex, color);
//
//        attributeDescriptions[2].binding = 0;
//        attributeDescriptions[2].location = 2;
//        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
//
//        return attributeDescriptions;
//    }
//
//    bool operator==(const Vertex& other) const
//	{
//        return pos == other.pos && color == other.color && texCoord == other.texCoord;
//    }
//};

namespace std
{
    template<> struct hash < gp2::Vertex >
	{
        size_t operator()(gp2::Vertex const& vertex) const
    	{
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}


struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class HelloTriangleApplication
{
public:
    void Run()
	{
        //InitWindow();
        InitVulkan();
        MainLoop();
        Cleanup();
    }

private:

    void CleanupVulkan()
    {
        CleanupSwapChain();

        vkDestroySampler(m_Device.GetLogicalDevice(), m_TextureSampler, nullptr);
        vkDestroyImageView(m_Device.GetLogicalDevice(), m_TextureImageView, nullptr);

        vkDestroyImage(m_Device.GetLogicalDevice(), m_TextureImage, nullptr);
        vkFreeMemory(m_Device.GetLogicalDevice(), m_TextureImageMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroyBuffer(m_Device.GetLogicalDevice(), m_UniformBuffers[i], nullptr);
            vkFreeMemory(m_Device.GetLogicalDevice(), m_UniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(m_Device.GetLogicalDevice(), m_DescriptorPool, nullptr);
        //vkDestroyDescriptorSetLayout(m_Device.GetLogicalDevice(), m_DescriptorSetLayout, nullptr);

        vkDestroyBuffer(m_Device.GetLogicalDevice(), m_IndexBuffer, nullptr);
        vkFreeMemory(m_Device.GetLogicalDevice(), m_IndexBufferMemory, nullptr);

        vkDestroyBuffer(m_Device.GetLogicalDevice(), m_VertexBuffer, nullptr);
        vkFreeMemory(m_Device.GetLogicalDevice(), m_VertexBufferMemory, nullptr);

        //vkDestroyPipeline(m_Device.GetLogicalDevice(), m_GraphicsPipeline, nullptr);
        //vkDestroyPipelineLayout(m_Device.GetLogicalDevice(), m_PipelineLayout, nullptr);

        //vkDestroyRenderPass(m_Device.GetLogicalDevice(), m_RenderPass.GetRenderPass(), nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            vkDestroySemaphore(m_Device.GetLogicalDevice(), m_RenderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_Device.GetLogicalDevice(), m_ImageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_Device.GetLogicalDevice(), m_InFlightFences[i], nullptr);
        }

        vkDestroyCommandPool(m_Device.GetLogicalDevice(), m_CommandPool, nullptr);
    }


    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
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

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
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

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        {
            return capabilities.currentExtent;
        }
        else 
        {
            int width, height;
            //glfwGetFramebufferSize(m_pWindow, &width, &height);
            width = m_Window.GetWidth();
            height = m_Window.GetHeight();

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

 //   void CreateSwapChain()
	//{
 //       gp2::Device::SwapChainSupportDetails swapChainSupport = m_Device.QuerySwapChainSupport(m_Device.GetPhysicalDevice());

 //       VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
 //       VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
 //       VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

 //       // Should not do plus one, change later.
 //       //uint32_t imageCount = swapChainSupport.capabilities.minImageCount;
 //       const uint32_t desiredImageCount = 2;
 //       uint32_t imageCount{};
 //       if (swapChainSupport.capabilities.maxImageCount > 0)
 //           imageCount = desiredImageCount;
 //       else
 //           imageCount = std::clamp(desiredImageCount, swapChainSupport.capabilities.minImageCount, swapChainSupport.capabilities.maxImageCount);

 //       /*uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
 //       if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) 
 //       {
 //           imageCount = swapChainSupport.capabilities.maxImageCount;
 //       }*/

 //       VkSwapchainCreateInfoKHR createInfo{};
 //       createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
 //       createInfo.surface = m_Device.GetSurface();
 //       createInfo.minImageCount = imageCount;
 //       createInfo.imageFormat = surfaceFormat.format;
 //       createInfo.imageColorSpace = surfaceFormat.colorSpace;
 //       createInfo.imageExtent = extent;
 //       createInfo.imageArrayLayers = 1;
 //       createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

 //       gp2::Device::QueueFamilyIndices indices = m_Device.FindQueueFamilies(m_Device.GetPhysicalDevice());
 //       uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

 //       if (indices.graphicsFamily != indices.presentFamily) 
 //       {
 //           createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
 //           createInfo.queueFamilyIndexCount = 2;
 //           createInfo.pQueueFamilyIndices = queueFamilyIndices;
 //       }
 //       else 
 //       {
 //           createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
 //           createInfo.queueFamilyIndexCount = 0; // Optional
 //           createInfo.pQueueFamilyIndices = nullptr; // Optional
 //       }

 //       createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
 //       createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
 //       createInfo.presentMode = presentMode;
 //       createInfo.clipped = VK_TRUE;
 //       createInfo.oldSwapchain = VK_NULL_HANDLE;

 //       if (vkCreateSwapchainKHR(m_Device.GetLogicalDevice(), &createInfo, nullptr, &m_SwapChain) != VK_SUCCESS) 
 //       {
 //           throw std::runtime_error("failed to create swap chain!");
 //       }

 //       vkGetSwapchainImagesKHR(m_Device.GetLogicalDevice(), m_SwapChain, &imageCount, nullptr);
 //       m_SwapChainImages.resize(imageCount);
 //       vkGetSwapchainImagesKHR(m_Device.GetLogicalDevice(), m_SwapChain, &imageCount, m_SwapChainImages.data());

 //       m_SwapChainImageFormat = surfaceFormat.format;
 //       m_SwapChainExtent = extent;
 //   }

 //   void CreateImageViews()
	//{
 //       m_SwapChainImageViews.resize(m_SwapChainImages.size());

 //       for (uint32_t i = 0; i < m_SwapChainImages.size(); i++)
 //       {
 //           m_SwapChainImageViews[i] = CreateImageView(m_SwapChainImages[i], m_SwapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
 //       }
 //   }

 //   void CreateGraphicsPipeline()
	//{
 //       auto vertShaderCode = ReadFile("shaders/shader_vert.spv");
 //       auto fragShaderCode = ReadFile("shaders/shader_frag.spv");

 //       VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
 //       VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

 //       VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
 //       vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
 //       vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
 //       vertShaderStageInfo.module = vertShaderModule;
 //       vertShaderStageInfo.pName = "main";

 //       VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
 //       fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
 //       fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
 //       fragShaderStageInfo.module = fragShaderModule;
 //       fragShaderStageInfo.pName = "main";

 //       VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

 //       VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
 //       inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
 //       inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
 //       inputAssembly.primitiveRestartEnable = VK_FALSE;

 //       VkViewport viewport{};
 //       viewport.x = 0.0f;
 //       viewport.y = 0.0f;
 //       viewport.width = (float)m_SwapChain.GetSwapChainExtent().width;
 //       viewport.height = (float)m_SwapChain.GetSwapChainExtent().height;
 //       viewport.minDepth = 0.0f;
 //       viewport.maxDepth = 1.0f;

 //       VkRect2D scissor{};
 //       scissor.offset = { 0, 0 };
 //       scissor.extent = m_SwapChain.GetSwapChainExtent();

 //       VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

 //       auto bindingDescription = gp2::Vertex::getBindingDescription();
 //       auto attributeDescriptions = gp2::Vertex::getAttributeDescriptions();

 //       vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
 //       vertexInputInfo.vertexBindingDescriptionCount = 1;
 //       vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
 //       vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
 //       vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


 //       std::vector<VkDynamicState> dynamicStates =
 //       {
 //           VK_DYNAMIC_STATE_VIEWPORT,
 //           VK_DYNAMIC_STATE_SCISSOR
 //       };

 //       VkPipelineDynamicStateCreateInfo dynamicState{};
 //       dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
 //       dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
 //       dynamicState.pDynamicStates = dynamicStates.data();

 //       VkPipelineViewportStateCreateInfo viewportState{};
 //       viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
 //       viewportState.viewportCount = 1;
 //       viewportState.scissorCount = 1;

 //       VkPipelineRasterizationStateCreateInfo rasterizer{};
 //       rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
 //       rasterizer.depthClampEnable = VK_FALSE;
 //       rasterizer.rasterizerDiscardEnable = VK_FALSE;
 //       rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
 //       rasterizer.lineWidth = 1.0f;
 //       rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
 //       rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
 //       rasterizer.depthBiasEnable = VK_FALSE;
 //       rasterizer.depthBiasConstantFactor = 0.0f; // Optional
 //       rasterizer.depthBiasClamp = 0.0f; // Optional
 //       rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

 //       VkPipelineMultisampleStateCreateInfo multisampling{};
 //       multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
 //       multisampling.sampleShadingEnable = VK_FALSE;
 //       multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
 //       multisampling.minSampleShading = 1.0f; // Optional
 //       multisampling.pSampleMask = nullptr; // Optional
 //       multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
 //       multisampling.alphaToOneEnable = VK_FALSE; // Optional

 //       VkPipelineDepthStencilStateCreateInfo depthStencil{};
 //       depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
 //       depthStencil.depthTestEnable = VK_TRUE;
 //       depthStencil.depthWriteEnable = VK_TRUE;
 //       depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
 //       depthStencil.depthBoundsTestEnable = VK_FALSE;
 //       depthStencil.minDepthBounds = 0.0f; // Optional
 //       depthStencil.maxDepthBounds = 1.0f; // Optional
 //       depthStencil.stencilTestEnable = VK_FALSE;
 //       depthStencil.front = {}; // Optional
 //       depthStencil.back = {}; // Optional

 //       VkPipelineColorBlendAttachmentState colorBlendAttachment{};
 //       colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
 //       colorBlendAttachment.blendEnable = VK_FALSE;
 //       colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
 //       colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
 //       colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
 //       colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
 //       colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
 //       colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

 //       VkPipelineColorBlendStateCreateInfo colorBlending{};
 //       colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
 //       colorBlending.logicOpEnable = VK_FALSE;
 //       colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
 //       colorBlending.attachmentCount = 1;
 //       colorBlending.pAttachments = &colorBlendAttachment;
 //       colorBlending.blendConstants[0] = 0.0f; // Optional
 //       colorBlending.blendConstants[1] = 0.0f; // Optional
 //       colorBlending.blendConstants[2] = 0.0f; // Optional
 //       colorBlending.blendConstants[3] = 0.0f; // Optional

 //   	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
 //       pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
 //       pipelineLayoutInfo.setLayoutCount = 0; // Optional
 //       pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
 //       pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
 //       //pipelineLayoutInfo.pSetLayouts = &m_DescriptorSetLayout;
	//	pipelineLayoutInfo.pSetLayouts = &m_Pipeline.GetDescriptorSetLayout();
 //       pipelineLayoutInfo.setLayoutCount = 1;

 //       if (vkCreatePipelineLayout(m_Device.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) 
 //       {
 //           throw std::runtime_error("failed to create pipeline layout!");
 //       }

 //       VkGraphicsPipelineCreateInfo pipelineInfo{};
 //       pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
 //       pipelineInfo.stageCount = 2;
 //       pipelineInfo.pStages = shaderStages;
 //       pipelineInfo.pVertexInputState = &vertexInputInfo;
 //       pipelineInfo.pInputAssemblyState = &inputAssembly;
 //       pipelineInfo.pViewportState = &viewportState;
 //       pipelineInfo.pRasterizationState = &rasterizer;
 //       pipelineInfo.pMultisampleState = &multisampling;
 //       pipelineInfo.pDepthStencilState = &depthStencil; // Optional
 //       pipelineInfo.pColorBlendState = &colorBlending;
 //       pipelineInfo.pDynamicState = &dynamicState;
 //       //pipelineInfo.layout = m_Pipeline.GetPipelineLayout();
 //       pipelineInfo.layout = m_PipelineLayout;
 //       pipelineInfo.renderPass = m_RenderPass.GetRenderPass();
 //       pipelineInfo.subpass = 0;
 //       pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
 //       pipelineInfo.basePipelineIndex = -1; // Optional

 //       if (vkCreateGraphicsPipelines(m_Device.GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) 
 //       {
 //           throw std::runtime_error("failed to create graphics pipeline!");
 //       }

 //       vkDestroyShaderModule(m_Device.GetLogicalDevice(), fragShaderModule, nullptr);
 //       vkDestroyShaderModule(m_Device.GetLogicalDevice(), vertShaderModule, nullptr);
 //   }

 //   void CreateRenderPass()
	//{
 //       VkAttachmentDescription colorAttachment{};
 //       colorAttachment.format = m_SwapChain.GetImageFormat();
 //       colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
 //       colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
 //       colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
 //       colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
 //       colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
 //       colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 //       colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

 //       VkAttachmentReference colorAttachmentRef{};
 //       colorAttachmentRef.attachment = 0;
 //       colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

 //       VkAttachmentDescription depthAttachment{};
 //       depthAttachment.format = FindDepthFormat();
 //       depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
 //       depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
 //       depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
 //       depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
 //       depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
 //       depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
 //       depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

 //       VkAttachmentReference depthAttachmentRef{};
 //       depthAttachmentRef.attachment = 1;
 //       depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

 //       VkSubpassDescription subpass{};
 //       subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
 //       subpass.colorAttachmentCount = 1;
 //       subpass.pColorAttachments = &colorAttachmentRef;
 //       subpass.pDepthStencilAttachment = &depthAttachmentRef;

 //       VkSubpassDependency dependency{};
 //       dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
 //       dependency.dstSubpass = 0;
 //       dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
 //       dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
 //       dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

 //       std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
 //       VkRenderPassCreateInfo renderPassInfo{};
 //       renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
 //       renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
 //       renderPassInfo.pAttachments = attachments.data();
 //       renderPassInfo.subpassCount = 1;
 //       renderPassInfo.pSubpasses = &subpass;
 //       renderPassInfo.dependencyCount = 1;
 //       renderPassInfo.pDependencies = &dependency;

 //       if (vkCreateRenderPass(m_Device.GetLogicalDevice(), &renderPassInfo, nullptr, &m_RenderPass) != VK_SUCCESS) 
 //       {
 //           throw std::runtime_error("failed to create render pass!");
 //       }
 //   }

 //   static std::vector<char> ReadFile(const std::string& filename)
	//{
 //       std::ifstream file(filename, std::ios::ate | std::ios::binary);

 //       if (!file.is_open()) 
 //       {
 //           throw std::runtime_error("failed to open file!");
 //       }

 //       size_t fileSize = (size_t)file.tellg();
 //       std::vector<char> buffer(fileSize);

 //       file.seekg(0);
 //       file.read(buffer.data(), fileSize);

 //       file.close();

 //       return buffer;
 //   }

 //   VkShaderModule CreateShaderModule(const std::vector<char>& code)
	//{
 //       VkShaderModuleCreateInfo createInfo{};
 //       createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
 //       createInfo.codeSize = code.size();
 //       createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

 //       VkShaderModule shaderModule;
 //       if (vkCreateShaderModule(m_Device.GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) 
 //       {
 //           throw std::runtime_error("failed to create shader module!");
 //       }

 //       return shaderModule;
 //   }

    void CreateFramebuffers()
	{
        m_SwapChainFramebuffers.resize(m_SwapChain.GetImageViews().size());

        for (size_t i = 0; i < m_SwapChain.GetImageViews().size(); i++)
        {
            std::array<VkImageView, 2> attachments = { m_SwapChain.GetImageViews()[i], m_DepthImageView };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_RenderPass.GetRenderPass();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_SwapChain.GetSwapChainExtent().width;
            framebufferInfo.height = m_SwapChain.GetSwapChainExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_Device.GetLogicalDevice(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("Failed to create framebuffer!");
            }
        }
    }

    void CreateCommandPool()
    {
        gp2::Device::QueueFamilyIndices queueFamilyIndices = m_Device.FindQueueFamilies(m_Device.GetPhysicalDevice());

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(m_Device.GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create command pool!");
        }
    }

    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0; // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_RenderPass.GetRenderPass();
        renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_SwapChain.GetSwapChainExtent();

        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        {
            vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetGraphicsPipeline());
            //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline);

            VkViewport viewport{};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_SwapChain.GetSwapChainExtent().width);
            viewport.height = static_cast<float>(m_SwapChain.GetSwapChainExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

            VkRect2D scissor{};
            scissor.offset = { 0, 0 };
            scissor.extent = m_SwapChain.GetSwapChainExtent();
            vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

            VkBuffer vertexBuffers[] = { m_VertexBuffer };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 1, &m_DescriptorSets[m_CurrentFrame], 0, nullptr);
            //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_PipelineLayout, 0, 1, &m_DescriptorSets[m_CurrentFrame], 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Indices.size()), 1, 0, 0, 0);
        }
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void CreateCommandBuffers()
	{
        m_CommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

        if (vkAllocateCommandBuffers(m_Device.GetLogicalDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void CreateSyncObjects()
	{
        m_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_RenderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        m_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            if (vkCreateSemaphore(m_Device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_Device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_Device.GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
            {

                throw std::runtime_error("failed to create synchronization objects for a frame!");
            }
        }
    }

    void DrawFrame()
	{
        vkWaitForFences(m_Device.GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_Device.GetLogicalDevice(), m_SwapChain.GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) 
        {
            RecreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        UpdateUniformBuffer(m_CurrentFrame);

        vkResetFences(m_Device.GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

        vkResetCommandBuffer(m_CommandBuffers[m_CurrentFrame], 0);
		RecordCommandBuffer(m_CommandBuffers[m_CurrentFrame], imageIndex);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_CommandBuffers[m_CurrentFrame];

        VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        VkSwapchainKHR swapChains[] = { m_SwapChain.GetSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr; // Optional

        result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
        {
            m_FramebufferResized = false;
            RecreateSwapChain();
        }
        else if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_CurrentFrame = (m_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void UpdateUniformBuffer(uint32_t currentImage)
	{
        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain.GetSwapChainExtent().width / (float)m_SwapChain.GetSwapChainExtent().height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    void CleanupSwapChain()
	{
        vkDestroyImageView(m_Device.GetLogicalDevice(), m_DepthImageView, nullptr);
        vkDestroyImage(m_Device.GetLogicalDevice(),m_DepthImage, nullptr);
        vkFreeMemory(m_Device.GetLogicalDevice(), m_DepthImageMemory, nullptr);

        for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++) 
        {
            vkDestroyFramebuffer(m_Device.GetLogicalDevice(), m_SwapChainFramebuffers[i], nullptr);
        }

        //for (size_t i = 0; i < m_SwapChainImageViews.size(); i++) 
        //{
        //    vkDestroyImageView(m_Device.GetLogicalDevice(), m_SwapChainImageViews[i], nullptr);
        //}

        //vkDestroySwapchainKHR(m_Device.GetLogicalDevice(), m_SwapChain, nullptr);
    }

    void RecreateSwapChain()
	{
        m_SwapChain.RecreateSwapChain();

        CreateDepthResources();
        CreateFramebuffers();
    }

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
	{
        auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
        app->m_FramebufferResized = true;
    }

    void CreateVertexBuffer()
	{
        VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device.GetLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_Vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_Device.GetLogicalDevice(), stagingBufferMemory);

        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_VertexBuffer, m_VertexBufferMemory);

        CopyBuffer(stagingBuffer, m_VertexBuffer, bufferSize);

        vkDestroyBuffer(m_Device.GetLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device.GetLogicalDevice(), stagingBufferMemory, nullptr);
    }

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
	{
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_Device.GetLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_Device.GetLogicalDevice(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_Device.FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device.GetLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(m_Device.GetLogicalDevice(), buffer, bufferMemory, 0);
    }

    //uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    //{
    //    VkPhysicalDeviceMemoryProperties memProperties;
    //    vkGetPhysicalDeviceMemoryProperties(m_vkPhysicalDevice, &memProperties);

    //    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
    //    {
    //        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
    //        {
    //            return i;
    //        }
    //    }

    //    throw std::runtime_error("failed to find suitable memory type!");
    //}

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
	{
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        EndSingleTimeCommands(commandBuffer);
    }

    void CreateIndexBuffer()
    {
        VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device.GetLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, m_Indices.data(), (size_t)bufferSize);
        vkUnmapMemory(m_Device.GetLogicalDevice(), stagingBufferMemory);

        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_IndexBuffer, m_IndexBufferMemory);

        CopyBuffer(stagingBuffer, m_IndexBuffer, bufferSize);

        vkDestroyBuffer(m_Device.GetLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device.GetLogicalDevice(), stagingBufferMemory, nullptr);
    }


    //void CreateDescriptorSetLayout()
    //{
    //    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    //    uboLayoutBinding.binding = 0;
    //    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    //    uboLayoutBinding.descriptorCount = 1;
    //    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    //    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    //    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    //    samplerLayoutBinding.binding = 1;
    //    samplerLayoutBinding.descriptorCount = 1;
    //    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //    samplerLayoutBinding.pImmutableSamplers = nullptr;
    //    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    //    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    //    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    //    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    //    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    //    layoutInfo.pBindings = bindings.data();

    //    if (vkCreateDescriptorSetLayout(m_Device.GetLogicalDevice(), &layoutInfo, nullptr, &m_Pipeline.GetDescriptorSetLayout()) != VK_SUCCESS)
    //        //if (vkCreateDescriptorSetLayout(m_Device.GetLogicalDevice(), &layoutInfo, nullptr, &m_DescriptorSetLayout) != VK_SUCCESS)
    //    {
    //        throw std::runtime_error("failed to create descriptor set layout!");
    //    }
    //}

    void CreateUniformBuffers()
    {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        m_UniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        m_UniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        m_UniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_UniformBuffers[i], m_UniformBuffersMemory[i]);

            vkMapMemory(m_Device.GetLogicalDevice(), m_UniformBuffersMemory[i], 0, bufferSize, 0, &m_UniformBuffersMapped[i]);
        }
    }

    void CreateDescriptorPool()
    {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        if (vkCreateDescriptorPool(m_Device.GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void CreateDescriptorSets()
    {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_Pipeline.GetDescriptorSetLayout());
        //std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_DescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_DescriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        m_DescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(m_Device.GetLogicalDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) 
        {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = m_TextureImageView;
            imageInfo.sampler = m_TextureSampler;


            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = m_DescriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = m_DescriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;

            vkUpdateDescriptorSets(m_Device.GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void CreateTextureImage()
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(m_Device.GetLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_Device.GetLogicalDevice(), stagingBufferMemory);

        stbi_image_free(pixels);

        CreateImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_TextureImage, m_TextureImageMemory);

        TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        CopyBufferToImage(stagingBuffer, m_TextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

        TransitionImageLayout(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(m_Device.GetLogicalDevice(), stagingBuffer, nullptr);
        vkFreeMemory(m_Device.GetLogicalDevice(), stagingBufferMemory, nullptr);

    }

    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
	{
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(m_Device.GetLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_Device.GetLogicalDevice(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = m_Device.FindMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_Device.GetLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(m_Device.GetLogicalDevice(), image, imageMemory, 0);
    }

    VkCommandBuffer BeginSingleTimeCommands()
	{
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_CommandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_Device.GetLogicalDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer)
	{
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_Device.GetGraphicsQueue());

        vkFreeCommandBuffers(m_Device.GetLogicalDevice(), m_CommandPool, 1, &commandBuffer);
    }

    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();


        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else 
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

            if (gp2::Device::HasStencilComponent(format)) 
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }
        else 
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }


        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        EndSingleTimeCommands(commandBuffer);
    }

    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
        VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        EndSingleTimeCommands(commandBuffer);
    }

    void CreateTextureImageView()
    {
        m_TextureImageView = CreateImageView(m_TextureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);

    }

    VkImageView CreateImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
	{
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(m_Device.GetLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create texture image view!");
        }

        return imageView;
    }

    void CreateTextureSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(m_Device.GetPhysicalDevice(), &properties);

        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_Device.GetLogicalDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

 //   bool HasStencilComponent(VkFormat format)
	//{
 //       return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
 //   }

    VkFormat FindDepthFormat()
	{
        return m_Device.FindSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    void CreateDepthResources()
	{
        VkFormat depthFormat = FindDepthFormat();

        CreateImage(m_SwapChain.GetSwapChainExtent().width, m_SwapChain.GetSwapChainExtent().height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DepthImage, m_DepthImageMemory);
        m_DepthImageView = CreateImageView(m_DepthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        TransitionImageLayout(m_DepthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

 /*   VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
	{
        for (VkFormat format : candidates) 
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_vkPhysicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) 
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) 
            {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }*/

    void LoadModel()
	{
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) 
        {
            throw std::runtime_error(warn + err);
        }

        std::unordered_map<gp2::Vertex, uint32_t> uniqueVertices{};


        for (const auto& shape : shapes) 
        {
            for (const auto& index : shape.mesh.indices) 
            {
                gp2::Vertex vertex{};

                vertex.pos = {
				    attrib.vertices[3 * index.vertex_index + 0],
				    attrib.vertices[3 * index.vertex_index + 1],
				    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.texCoord = {
				    attrib.texcoords[2 * index.texcoord_index + 0],
				    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = { 1.0f, 1.0f, 1.0f };

                if (uniqueVertices.count(vertex) == 0) 
                {
                    uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
                    m_Vertices.push_back(vertex);
                }

                m_Indices.push_back(uniqueVertices[vertex]);
            }
        }
    }

    void InitVulkan()
	{
        // Intstance
        //CreateInstance();
        //SetupDebugMessenger();

		// Device
        //CreateSurface();
        //PickPhysicalDevice();
        //CreateLogicalDevice();

		// Swap Chain
        //CreateSwapChain();
        //CreateImageViews();
        //CreateRenderPass();

		// Pipeline
        //CreateDescriptorSetLayout();
        //CreateGraphicsPipeline();

        // Device
        CreateCommandPool();
        CreateDepthResources();
        CreateFramebuffers();

        // Texture
        CreateTextureImage();
        CreateTextureImageView();
        CreateTextureSampler();

		// Model 
        LoadModel();
    	CreateVertexBuffer();
        CreateIndexBuffer();
        CreateUniformBuffers();

		// Descriptor
        CreateDescriptorPool();
        CreateDescriptorSets();

        CreateCommandBuffers();
        CreateSyncObjects();
    }

    void MainLoop()
	{
        while(!m_Window.ShouldClose()) 
        {
            glfwPollEvents();
            DrawFrame();
        }
        vkDeviceWaitIdle(m_Device.GetLogicalDevice());
    }

    void Cleanup()
	{
        CleanupVulkan();

        glfwTerminate();
    }


    gp2::Window m_Window{};

    gp2::Device m_Device{&m_Window};
    gp2::SwapChain m_SwapChain{ &m_Window, &m_Device };

	gp2::RenderPass m_RenderPass{ &m_Device, &m_SwapChain };
	gp2::Pipeline m_Pipeline{ &m_Device, &m_SwapChain ,&m_RenderPass, "shaders/shader_vert.spv", "shaders/shader_frag.spv" };


    //std::vector<VkImage> m_SwapChainImages;
    //VkFormat m_SwapChainImageFormat{};
    //VkExtent2D m_SwapChainExtent{};

    //std::vector<VkImageView> m_SwapChainImageViews;
    //VkRenderPass m_RenderPass;

    //VkPipelineLayout m_PipelineLayout;
    //VkPipeline m_GraphicsPipeline;
    //VkDescriptorSetLayout m_DescriptorSetLayout;

    std::vector<VkFramebuffer> m_SwapChainFramebuffers;
    VkCommandPool m_CommandPool;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;

    bool m_FramebufferResized = false;
    uint32_t m_CurrentFrame = 0;

    std::vector<gp2::Vertex> m_Vertices;
    std::vector<uint32_t> m_Indices;

    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;
    VkBuffer m_IndexBuffer;
    VkDeviceMemory m_IndexBufferMemory;


    std::vector<VkBuffer> m_UniformBuffers;
    std::vector<VkDeviceMemory> m_UniformBuffersMemory;
    std::vector<void*> m_UniformBuffersMapped;

    VkDescriptorPool m_DescriptorPool;
    std::vector<VkDescriptorSet> m_DescriptorSets;

    VkImage m_TextureImage;
    VkDeviceMemory m_TextureImageMemory;
    VkImageView m_TextureImageView;
    VkSampler m_TextureSampler;

    VkImage m_DepthImage;
    VkDeviceMemory m_DepthImageMemory;
    VkImageView m_DepthImageView;
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

 