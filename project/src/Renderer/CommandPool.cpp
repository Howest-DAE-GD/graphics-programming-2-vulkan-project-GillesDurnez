#include "CommandPool.h"

#include <stdexcept>

gp2::Commandpool::Commandpool(Device* pDevice)
	: m_pDevice(pDevice)
{
	CreateCommandPool();
}

gp2::Commandpool::~Commandpool()
{
	vkDestroyCommandPool(m_pDevice->GetLogicalDevice(), m_CommandPool, nullptr);
}

VkCommandBuffer gp2::Commandpool::BeginSingleTimeCommands() const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_pDevice->GetLogicalDevice(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void gp2::Commandpool::EndSingleTimeCommands(VkCommandBuffer commandBuffer) const
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_pDevice->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_pDevice->GetGraphicsQueue());

	vkFreeCommandBuffers(m_pDevice->GetLogicalDevice(), m_CommandPool, 1, &commandBuffer);
}

void gp2::Commandpool::CreateCommandPool()
{
	Device::QueueFamilyIndices queueFamilyIndices = m_pDevice->FindQueueFamilies();

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

	if (vkCreateCommandPool(m_pDevice->GetLogicalDevice(), &poolInfo, nullptr, &m_CommandPool) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create command pool!");
	}
}
