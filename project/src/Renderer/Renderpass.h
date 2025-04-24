#pragma once
#include <vulkan/vulkan.h>

#include "Device.h"
#include "SwapChain.h"

namespace gp2
{
	class RenderPass
	{
	public:
		RenderPass(Device* pDevice, SwapChain* pSwapChain);
		~RenderPass();

		RenderPass(const RenderPass&) = delete;
		RenderPass(RenderPass&&) = delete;
		RenderPass& operator=(const RenderPass&) = delete;
		RenderPass& operator=(RenderPass&&) = delete;

		VkRenderPass GetRenderPass() const { return m_RenderPass; }
	private:
		void CreateRenderPass();
		VkFormat FindDepthFormat() const;


		Device* m_pDevice;
		SwapChain* m_SwapChain;

		VkRenderPass m_RenderPass;
	};
}
