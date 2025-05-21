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

	class BaseRenderPass
	{
	public:
		BaseRenderPass(Device* pDevice, CommandPool* pCommandPool, SwapChain* pSwapChain, VkSampler sampler);


		~BaseRenderPass();

		void CreateDescriptorSets(Scene* pScene);

		void RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene) const;

		void Update(Camera* pCamera, uint32_t currentImage) const;

	private:
		std::vector<VkDescriptorSetLayout> CreateDescriptorSetLayout() const;
		VkDescriptorPool CreateDescriptorPool() const;
		void CreateUniformBuffers();


		Device* m_pDevice;
		CommandPool* m_pCommandPool;
		SwapChain* m_pSwapChain;

		VkSampler m_TextureSampler;

		DescriptorPool m_DescriptorPool{ m_pDevice, CreateDescriptorSetLayout() };

		std::vector<VkDescriptorSet> m_UBODescriptorSets{};
		VkDescriptorSet m_TextureDescriptorSet{};

		std::vector<Buffer> m_UniformBuffers;

		Pipeline m_Pipeline{ m_pDevice, m_pSwapChain, &m_DescriptorPool , "shaders/shader_vert.spv", "shaders/shader_frag.spv" };
	};
}
