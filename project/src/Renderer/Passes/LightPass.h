#pragma once
#include <vulkan/vulkan_core.h>

#include "BaseRenderPass.h"
#include "Camera.h"
#include "Image.h"
#include "Pipeline.h"
#include "Scene.h"
#include "SwapChain.h"

namespace gp2
{
	class LightPass
	{
	public:
		LightPass(Device* pDevice, CommandPool* pCommandPool, SwapChain* pSwapChain, GBuffer* pGBuffer,  Image* pDepthImage, VkSampler sampler);

		LightPass(const LightPass& other) = delete;
		LightPass(LightPass&& other) noexcept = delete;
		LightPass& operator=(const LightPass& other) = delete;
		LightPass& operator=(LightPass&& other) noexcept = delete;

		~LightPass();

		void CreateDescriptorSets(Scene* pScene);

		void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene, Image* depthImage, Image* targetImage) const;

		void Update(Camera* pCamera, uint32_t currentImage) const;

	private:
		std::vector<VkDescriptorSetLayout> CreateDescriptorSetLayout() const;
		VkDescriptorPool CreateDescriptorPool() const;
		void CreateUniformBuffers();
		Pipeline::PipelineConfig& CreatePipeLineConfig(Image* pDepthImage);

		Device* m_pDevice;
		CommandPool* m_pCommandPool;
		SwapChain* m_pSwapChain;

		VkSampler m_TextureSampler;

		GBuffer* m_pGBuffer{};
		Image* m_pDepthImage{};

		DescriptorPool m_DescriptorPool{ m_pDevice, CreateDescriptorSetLayout() };

		VkDescriptorSet m_GBufferDescriptorSet{};

		std::vector<Buffer> m_UniformBuffers;

		Pipeline::PipelineConfig m_PipelineConfig{};
		Pipeline m_Pipeline;
	};
}
