#include "Buffer.h"

#include <stdexcept>

gp2::Buffer::Buffer(Device* pDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties)
	: m_pDevice(pDevice)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_pDevice->GetLogicalDevice(), &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_pDevice->GetLogicalDevice(), m_Buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_pDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_pDevice->GetLogicalDevice(), &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_pDevice->GetLogicalDevice(), m_Buffer, m_BufferMemory, 0);
}

gp2::Buffer::Buffer(Device* pDevice, VkBufferCreateInfo bufferInfo, VkMemoryPropertyFlags properties)
{
    if (vkCreateBuffer(m_pDevice->GetLogicalDevice(), &bufferInfo, nullptr, &m_Buffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer!");
    }

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_pDevice->GetLogicalDevice(), m_Buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = m_pDevice->FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(m_pDevice->GetLogicalDevice(), &allocInfo, nullptr, &m_BufferMemory) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate buffer memory!");
    }

    vkBindBufferMemory(m_pDevice->GetLogicalDevice(), m_Buffer, m_BufferMemory, 0);
}

gp2::Buffer::~Buffer()
{
	if (m_Buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_pDevice->GetLogicalDevice(), m_Buffer, nullptr);
	}
	if (m_BufferMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_pDevice->GetLogicalDevice(), m_BufferMemory, nullptr);
	}
}

void gp2::Buffer::CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size) const
{
    VkCommandBuffer commandBuffer = m_pDevice->BeginSingleTimeCommands();

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, m_Buffer, 1, &copyRegion);

    m_pDevice->EndSingleTimeCommands(commandBuffer);
}
