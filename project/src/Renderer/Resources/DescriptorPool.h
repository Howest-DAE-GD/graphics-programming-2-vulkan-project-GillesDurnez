#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

#include "CommandPool.h"
#include "Device.h"

namespace gp2
{
	class DescriptorPool
	{
	public:

		struct DescriptorSetLayoutData
		{
			std::vector<VkDescriptorSetLayoutBinding>	bindings;
			VkDescriptorSetLayoutCreateInfo				info{};
		};

		DescriptorPool(Device* pDevice, const std::vector<VkDescriptorSetLayout>& layoutInfos);

		DescriptorPool(const DescriptorPool& other) = delete;
		DescriptorPool(DescriptorPool&& other) noexcept = delete;
		DescriptorPool& operator=(const DescriptorPool& other) = delete;
		DescriptorPool& operator=(DescriptorPool&& other) noexcept = delete;

		~DescriptorPool();

		void SetDescriptorSetPool(VkDescriptorPool poolInfo);

		const VkDescriptorSetLayout& GetDescriptorSetLayout(int index) const;
		const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayouts() const;
		const VkDescriptorPool& GetDescriptorPool() const;
		


		static constexpr uint32_t MAX_POOL_RESERVE = 512;
	private:

		void CreateDescriptorSetLayouts(const std::vector<VkDescriptorSetLayout>& layoutInfos);
		void CreateDescriptorPool( VkDescriptorPool poolInfo);

		Device* m_pDevice{};

		VkDescriptorPool m_DescriptorPool{};
		std::vector<VkDescriptorSetLayout> m_DescriptorSetLayouts{};
	};
}
