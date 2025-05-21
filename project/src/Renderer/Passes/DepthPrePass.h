#pragma once

#include "Camera.h"
#include "DescriptorPool.h"
#include "Pipeline.h"
#include "Scene.h"

namespace gp2
{
	class DepthPrePass
	{
	public:
		DepthPrePass(Device* pDevice, CommandPool* pCommandPool, SwapChain* pSwapChain);

		DepthPrePass(const DepthPrePass& other) = delete;
		DepthPrePass(DepthPrePass&& other) noexcept = delete;
		DepthPrePass& operator=(const DepthPrePass& other) = delete;
		DepthPrePass& operator=(DepthPrePass&& other) noexcept = delete;
		
		~DepthPrePass();

		Image* ReCreateDepthResource();
		Image* CreateDepthResource();
		Image* GetDepthImage() const { return m_pDepthImage; }

		void CreateDescriptorSets(Scene* pScene);
		void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene) const;
		void Update(Camera* pCamera, uint32_t currentImage) const;

	private:

		std::vector<VkDescriptorSetLayout> CreateDescriptorSetLayout() const;
		VkDescriptorPool CreateDescriptorPool() const;
		void CreateUniformBuffers();
		Pipeline::PipelineConfig& CreatePipeLineConfig();

		Device* m_pDevice;
		CommandPool* m_pCommandPool;
		SwapChain* m_pSwapChain;

		DescriptorPool m_DescriptorPool{ m_pDevice, CreateDescriptorSetLayout() };

		std::vector<VkDescriptorSet> m_UBODescriptorSets{};

		std::vector<Buffer> m_UniformBuffers;

		Image* m_pDepthImage{ CreateDepthResource() };

		Pipeline::PipelineConfig m_PipelineConfig{};
		Pipeline m_Pipeline{ m_pDevice, m_pSwapChain, &m_DescriptorPool, CreatePipeLineConfig() , "shaders/depthPrePass_vert.spv", "" };
	};
}
