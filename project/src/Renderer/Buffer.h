#pragma once
#include "CommandPool.h"
#include "Device.h"

namespace gp2
{
	class Buffer
	{
	public:

		Buffer(Device* pDevice, CommandPool* pCommandPool, const VkBufferCreateInfo& bufferInfo, VkMemoryPropertyFlags properties, bool hostAcces = false);
		~Buffer();

		Buffer(Buffer&) = delete;
		Buffer(Buffer&&) = default;
		Buffer& operator=(Buffer&) = delete;
		Buffer& operator=(Buffer&&) = default;

		VkBuffer GetBuffer() const { return m_Buffer; }
		VmaAllocation GetBufferAllocation() const { return m_BufferAllocation; }

		void CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size) const;


	private:
		Device* m_pDevice;
		CommandPool* m_pCommandPool;

		VkBuffer m_Buffer{};
		VmaAllocation m_BufferAllocation{};
	};
}
