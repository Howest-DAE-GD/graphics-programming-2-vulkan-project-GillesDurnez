#pragma once
#include "Defines.h"

#include <vector>
#include <vulkan/vulkan_core.h>

#include "BaseRenderPass.h"
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
#include "DepthPrePass.h"
#include "DescriptorPool.h"
#include "LightPass.h"

namespace gp2
{

	class VkRenderer
	{
	public:
		VkRenderer();
        ~VkRenderer();
		//void UpdateAllTextureDescriptors();

        void Update();
		void RenderFrame();

        Window& GetWindow() { return m_Window; }

	private:
        // Setup
		//void CreateFrameBuffers();
		VkSampler CreateTextureSampler();
        //void CreateUniformBuffers();

        void CreateSyncObjects();

        // Cleanup
        //void CleanupSwapChain() const;

        // Per Frame
        void RecreateSwapChain();
        void UpdateUniformBuffer(uint32_t currentImage);
        void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);



        Window m_Window{};

        Device m_Device{ &m_Window };
        CommandPool m_CommandPool{ &m_Device };

        SwapChain m_SwapChain{ &m_Window, &m_Device, &m_CommandPool };
        VkSampler m_TextureSampler{ CreateTextureSampler() };

        DepthPrePass m_DepthPrePass{ &m_Device, &m_CommandPool, &m_SwapChain };
        BaseRenderPass m_BaseRenderPass{ &m_Device, &m_CommandPool, &m_SwapChain, m_DepthPrePass.GetDepthImage() ,m_TextureSampler };
		LightPass m_LightPass{ &m_Device, &m_CommandPool, &m_SwapChain, &m_BaseRenderPass, &m_DepthPrePass, m_TextureSampler };

		Scene m_Scene{};
		Camera m_Camera{ &m_Window,  m_SwapChain.GetSwapChainExtent().width / static_cast<float>(m_SwapChain.GetSwapChainExtent().height),{ 0.f, 0.f, 0.f }, 45.f, .1f, 10000.f };

        // Renderer
        //std::vector<VkFramebuffer> m_SwapChainFramebuffers;

        std::vector<VkSemaphore> m_ImageAvailableSemaphores;
        std::vector<VkSemaphore> m_RenderFinishedSemaphores;
        std::vector<VkFence> m_InFlightFences;

        bool m_FramebufferResized = false;
        uint32_t m_CurrentFrame = 0;

        int frameCount = 0;
	};
	
}
