#include "DescriptorPool.h"

#include <array>

#include "Pipeline.h"
#include "SwapChain.h"

gp2::DescriptorPool::DescriptorPool(Device* pDevice, const std::vector<VkDescriptorSetLayout>& layoutInfos)
	: m_pDevice(pDevice)
{
	CreateDescriptorSetLayouts(layoutInfos);
	//CreateDescriptorPool(poolInfo);
}

gp2::DescriptorPool::~DescriptorPool()
{
	vkDestroyDescriptorPool(m_pDevice->GetLogicalDevice(), m_DescriptorPool, nullptr);

	for (int index{}; index < m_DescriptorSetLayouts.size(); ++index)
	{
		vkDestroyDescriptorSetLayout(m_pDevice->GetLogicalDevice(), m_DescriptorSetLayouts[index], nullptr);
	}
}

void gp2::DescriptorPool::SetDescriptorSetPool(VkDescriptorPool poolInfo)
{
	CreateDescriptorPool(poolInfo);
}

const VkDescriptorSetLayout& gp2::DescriptorPool::GetDescriptorSetLayout(int index) const
{
	return m_DescriptorSetLayouts[index];
}

const std::vector<VkDescriptorSetLayout>& gp2::DescriptorPool::GetDescriptorSetLayouts() const
{
	return m_DescriptorSetLayouts;
}

const VkDescriptorPool& gp2::DescriptorPool::GetDescriptorPool() const
{
	return m_DescriptorPool;
}

void gp2::DescriptorPool::CreateDescriptorSetLayouts(const std::vector<VkDescriptorSetLayout>& layoutInfos)
{

	m_DescriptorSetLayouts = layoutInfos;
	//m_DescriptorSetLayouts.resize(layoutInfos.size());
	//for (int index{}; index < layoutInfos.size(); ++index)
	//{
	//	auto layoutInfo = layoutInfos[index];
	//	if (vkCreateDescriptorSetLayout(
	//		m_pDevice->GetLogicalDevice(),
	//		&layoutInfo.info,
	//		nullptr,
	//		&m_DescriptorSetLayouts[index]) != VK_SUCCESS)
	//	{
	//		throw std::runtime_error("Failed to create descriptor set layout" + index);
	//	}
	//}
}

void gp2::DescriptorPool::CreateDescriptorPool(VkDescriptorPool poolInfo)
{
	m_DescriptorPool = poolInfo;
	//if (vkCreateDescriptorPool(
	//	m_pDevice->GetLogicalDevice(),
	//	&poolInfo,
	//	nullptr,
	//	&m_DescriptorPool) != VK_SUCCESS)
	//{
	//	throw std::runtime_error("failed to create descriptor pool!");
	//}
}
