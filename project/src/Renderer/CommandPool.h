#pragma once
#include <vulkan/vulkan.h>
#include "Device.h"

namespace gp2
{
	class CommandPool
	{
	public:
		CommandPool(Device* pDevice);
		~CommandPool();

		VkCommandBuffer BeginSingleTimeCommands() const;
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;

		VkCommandPool GetCommandPool() const { return m_CommandPool; }
		std::vector<VkCommandBuffer>& GetCommandBuffers() { return m_CommandBuffers; }

	private:
		Device* m_pDevice;

		VkCommandPool m_CommandPool;
		std::vector<VkCommandBuffer> m_CommandBuffers;

		void CreateCommandPool();
		void CreateCommandBuffers();

	};
}