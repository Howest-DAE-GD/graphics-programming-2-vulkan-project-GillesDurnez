#pragma once
#include <vulkan/vulkan.h>

#include "SwapChain.h"
#include "Device.h"
#include "RenderPass.h"

namespace gp2
{

	///The pipeline\n
	/// Specifies the information of the different stages in the graphics pipeline
	///
	class Pipeline
	{
	public:
		Pipeline(Device* device, SwapChain* swapChain, RenderPass* pRenderPass, const std::string& vertShaderPath, const std::string& fragShaderPath);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline(Pipeline&&) = delete;
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline& operator=(Pipeline&&) = delete;

		VkDescriptorSetLayout& GetDescriptorSetLayout() { return m_DescriptorSetLayout; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
		VkPipeline GetGraphicsPipeline() const { return m_GraphicsPipeline; }

	private:

		void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline(const std::string& vertShaderPath, const std::string& fragShaderPath);

		Device* m_pDevice;
		SwapChain* m_pSwapChain;
		RenderPass* m_pRenderPass;

		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		VkDescriptorSetLayout m_DescriptorSetLayout;

	};
}
