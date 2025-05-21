#pragma once
#include <vulkan/vulkan.h>

#include "DescriptorPool.h"
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
		struct PipelineConfig
		{
			VkPipelineDepthStencilStateCreateInfo depthStencil{};
			Image* depthImage{};
			VkPipelineRenderingCreateInfo renderInfo{};
			std::vector<VkPushConstantRange> pushConstants{};
		};

		//Pipeline(Device* device, SwapChain* swapChain, RenderPass* pRenderPass, const std::string& vertShaderPath, const std::string& fragShaderPath);
		Pipeline(Device* device, SwapChain* swapChain, DescriptorPool* pDescriptorPool, const PipelineConfig& pipelineConfig,const std::string& vertShaderPath, const std::string& fragShaderPath);
		~Pipeline();

		Pipeline(const Pipeline&) = delete;
		Pipeline(Pipeline&&);
		Pipeline& operator=(const Pipeline&) = delete;
		Pipeline& operator=(Pipeline&&);

		//VkDescriptorSetLayout& GetDescriptorSetLayout(int index) { return m_DescriptorSetLayout[index]; }
		VkPipelineLayout GetPipelineLayout() const { return m_PipelineLayout; }
		VkPipeline GetGraphicsPipeline() const { return m_GraphicsPipeline; }

	private:

		//void CreateDescriptorSetLayout();
		void CreateGraphicsPipeline(const PipelineConfig& pipelineConfig, const std::string& vertShaderPath, const std::string& fragShaderPath);
		DescriptorPool* m_pDescriptorPool{};

		Device* m_pDevice{};
		SwapChain* m_pSwapChain{};
		RenderPass* m_pRenderPass{};

		VkPipelineLayout m_PipelineLayout;
		VkPipeline m_GraphicsPipeline;
		//std::vector<VkDescriptorSetLayout> m_DescriptorSetLayout;


	};
}
