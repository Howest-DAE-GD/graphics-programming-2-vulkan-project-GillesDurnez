#pragma once
#include "Defines.h"

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Window.h"
#include "Device.h"
#include "CommandPool.h"
#include "SwapChain.h"
#include "Renderpass.h"
#include "Pipeline.h"
#include "Resources/Texture.h"
#include "Resources/Model.h"
#include "Resources/Scene.h"
#include "Camera.h"

namespace gp2
{
    struct UniformBufferObject
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 view;
        alignas(16) glm::mat4 proj;
    };


	class VkRenderer
	{
	public:
		VkRenderer();
        ~VkRenderer();

        void RenderFrame();

        Window& GetWindow() { return m_Window; }

	private:
        // Setup
		//void CreateFrameBuffers();
		void CreateTextureSampler();
        void CreateUniformBuffers();
        void CreateDescriptorPool();
        void CreateDescriptorSets();
        void CreateSyncObjects();

        // Cleanup
        //void CleanupSwapChain() const;

        // Per Frame
        void RecreateSwapChain();
        void UpdateUniformBuffer(uint32_t currentImage) const;
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);


        Window m_Window{};

        Device m_Device{ &m_Window };
        CommandPool m_CommandPool{ &m_Device };

        SwapChain m_SwapChain{ &m_Window, &m_Device, &m_CommandPool };

        //RenderPass m_RenderPass{ &m_Device, &m_SwapChain };
        Pipeline m_Pipeline{ &m_Device, &m_SwapChain , "shaders/shader_vert.spv", "shaders/shader_frag.spv" };

        // Todo move outside of renderer
		Scene m_Scene{};
		Camera m_Camera{ &m_Window,  m_SwapChain.GetSwapChainExtent().width / static_cast<float>(m_SwapChain.GetSwapChainExtent().height),{ 0.f, 0.f, 0.f }, 45.f, .1f, 10000.f };

        // Renderer
        //std::vector<VkFramebuffer> m_SwapChainFramebuffers;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        bool m_FramebufferResized = false;
        uint32_t m_CurrentFrame = 0;

        // Renderer
        std::vector<Buffer> m_UniformBuffers;

        // Renderer
        VkDescriptorPool m_DescriptorPool{};
        std::vector<VkDescriptorSet> m_DescriptorSets;

        VkSampler m_TextureSampler{};
	};
	
}
