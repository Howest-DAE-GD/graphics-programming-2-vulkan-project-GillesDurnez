#pragma once
#include <vulkan/vulkan.h>
#include "Device.h"

namespace gp2
{
	class Commandpool
	{
	public:
		Commandpool(Device* pDevice);
		~Commandpool();

		VkCommandBuffer BeginSingleTimeCommands() const;
		void EndSingleTimeCommands(VkCommandBuffer commandBuffer) const;

	private:
		Device* m_pDevice;

		VkCommandPool m_CommandPool;

		void CreateCommandPool();
	};
}