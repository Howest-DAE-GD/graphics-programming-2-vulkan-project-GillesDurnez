#pragma once
#include <vector>

#include "Camera.h"
#include "DescriptorPool.h"
#include "Pipeline.h"
#include "Scene.h"

namespace gp2
{
	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	struct GBuffer
	{
		Image diffuse{};
		Image normal{};
		Image metalnessAndRoughness{};
	};

	class BaseRenderPass
	{
	public:
		BaseRenderPass(Device* pDevice, CommandPool* pCommandPool, SwapChain* pSwapChain, Image* pDepthImage, VkSampler sampler);

		BaseRenderPass(const BaseRenderPass& other) = delete;
		BaseRenderPass(BaseRenderPass&& other) noexcept = delete;
		BaseRenderPass& operator=(const BaseRenderPass& other) = delete;
		BaseRenderPass& operator=(BaseRenderPass&& other) noexcept = delete;

		~BaseRenderPass();

		void CreateDescriptorSets(Scene* pScene);

		void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene, Image* depthImage, Image* targetImage);

		void Update(Camera* pCamera, uint32_t currentImage) const;

		void RecreateGBuffer();

		GBuffer* GetGBuffer() const { return m_GBuffer; }

	private:
		std::vector<VkDescriptorSetLayout> CreateDescriptorSetLayout() const;
		VkDescriptorPool CreateDescriptorPool() const;
		void CreateUniformBuffers();
		Pipeline::PipelineConfig& CreatePipeLineConfig(Image* pDepthImage);

		GBuffer* CreateGBuffer();

		Device* m_pDevice;
		CommandPool* m_pCommandPool;
		SwapChain* m_pSwapChain;

		VkSampler m_TextureSampler;

		GBuffer* m_GBuffer{CreateGBuffer()};

		DescriptorPool m_DescriptorPool{ m_pDevice, CreateDescriptorSetLayout() };

		std::vector<VkDescriptorSet> m_UBODescriptorSets{};
		VkDescriptorSet m_TextureDescriptorSet{};

		std::vector<Buffer> m_UniformBuffers;

		Pipeline::PipelineConfig m_PipelineConfig{};
		Pipeline m_Pipeline;
	};
}
