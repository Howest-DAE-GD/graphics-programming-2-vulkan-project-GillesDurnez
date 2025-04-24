#pragma once
#include "Device.h"

namespace gp2
{
	class Buffer
	{
	public:

		Buffer(Device* pDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
		Buffer(Device* pDevice, VkBufferCreateInfo bufferInfo, VkMemoryPropertyFlags properties);
		~Buffer();

		Buffer(Buffer&) = delete;
		Buffer(Buffer&&) = default;
		Buffer& operator=(Buffer&) = delete;
		Buffer& operator=(Buffer&&) = default;

		VkBuffer GetBuffer() const { return m_Buffer; }
		VkDeviceMemory GetBufferMemory() const { return m_BufferMemory; }

		void CopyBuffer(VkBuffer srcBuffer, VkDeviceSize size) const;


	private:
		Device* m_pDevice;

		VkBuffer m_Buffer{};
		VkDeviceMemory m_BufferMemory{};
	};
}
