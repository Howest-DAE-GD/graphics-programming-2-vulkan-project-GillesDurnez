#include "Buffer.h"

#include <stdexcept>

//#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

gp2::Buffer::Buffer(Device* pDevice, CommandPool* pCommandPool, const VkBufferCreateInfo& bufferInfo, const VkMemoryPropertyFlags properties, bool hostAcces)
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool)
{
	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.requiredFlags = properties;

	if (hostAcces)
	{
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT;
	}

	vmaCreateBuffer(pDevice->GetAllocator(), &bufferInfo, &allocInfo, &m_Buffer, &m_BufferAllocation, nullptr);
	vmaSetAllocationName(pDevice->GetAllocator(), m_BufferAllocation, "MyBuffer");
}

gp2::Buffer::~Buffer()
{
	vmaDestroyBuffer(m_pDevice->GetAllocator(), m_Buffer, m_BufferAllocation);
}

void gp2::Buffer::CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size) const
{
    VkCommandBuffer commandBuffer = m_pCommandPool->BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, m_Buffer, 1, &copyRegion);

    m_pCommandPool->EndSingleTimeCommands(commandBuffer);
}
