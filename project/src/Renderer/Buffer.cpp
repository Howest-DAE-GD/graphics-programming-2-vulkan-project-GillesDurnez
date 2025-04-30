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
	if (m_Buffer != VK_NULL_HANDLE && m_BufferAllocation != VK_NULL_HANDLE)
		vmaDestroyBuffer(m_pDevice->GetAllocator(), m_Buffer, m_BufferAllocation);
}

gp2::Buffer::Buffer(Buffer&& other)
{
	m_Buffer = other.m_Buffer;
	m_BufferAllocation = other.m_BufferAllocation;
	m_pDevice = other.m_pDevice;
	m_pCommandPool = other.m_pCommandPool;
	other.m_Buffer = VK_NULL_HANDLE;
	other.m_BufferAllocation = VK_NULL_HANDLE;
}

gp2::Buffer& gp2::Buffer::operator=(Buffer&& other)
{
	m_Buffer = other.m_Buffer;
	m_BufferAllocation = other.m_BufferAllocation;
	m_pDevice = other.m_pDevice;
	m_pCommandPool = other.m_pCommandPool;
	other.m_Buffer = VK_NULL_HANDLE;
	other.m_BufferAllocation = VK_NULL_HANDLE;

	return *this;
}

void gp2::Buffer::CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size) const
{
    VkCommandBuffer commandBuffer = m_pCommandPool->BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, m_Buffer, 1, &copyRegion);

    m_pCommandPool->EndSingleTimeCommands(commandBuffer);
}

void gp2::Buffer::CopyMemory(void* data, const VkDeviceSize& size, int offset) const
{
	vmaCopyMemoryToAllocation(m_pDevice->GetAllocator(), data, m_BufferAllocation, offset, size);
}
