#include "Pipeline.h"

#include <array>
#include <stdexcept>

#include "Resources/Model.h"
#include "Resources/Shader.h"

gp2::Pipeline::Pipeline(Device* device, SwapChain* swapChain, RenderPass* pRenderPass,const std::string& vertShaderPath, const std::string& fragShaderPath)
	: m_pDevice(device), m_pSwapChain(swapChain), m_pRenderPass(pRenderPass)
{
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline(vertShaderPath, fragShaderPath);
}

gp2::Pipeline::Pipeline(Device* device, SwapChain* swapChain, const std::string& vertShaderPath,
	const std::string& fragShaderPath)
	: m_pDevice(device), m_pSwapChain(swapChain)
{
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline(vertShaderPath, fragShaderPath);
}

gp2::Pipeline::~Pipeline()
{
	vkDestroyPipeline(m_pDevice->GetLogicalDevice(), m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_pDevice->GetLogicalDevice(), m_PipelineLayout, nullptr);

	for (int index{}; index < m_DescriptorSetLayout.size(); ++index)
	{
		vkDestroyDescriptorSetLayout(m_pDevice->GetLogicalDevice(), m_DescriptorSetLayout[index], nullptr);
	}
}

void gp2::Pipeline::CreateDescriptorSetLayout()
{
	m_DescriptorSetLayout.resize(2);
	// UBO DescriptorSet
	VkDescriptorSetLayoutBinding uboBinding{};
	uboBinding.binding = 0;
	uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboBinding.descriptorCount = 1;
	uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboBinding.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo layoutInfo0{};
	layoutInfo0.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo0.bindingCount = 1;
	layoutInfo0.pBindings = &uboBinding;

	if (vkCreateDescriptorSetLayout(
		m_pDevice->GetLogicalDevice(),
		&layoutInfo0,
		nullptr,
		&m_DescriptorSetLayout[0]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create UBO descriptor set layout!");
	}

	// Create texture descriptor sets
	constexpr uint32_t MAX_TEXTURES = 512;
	VkDescriptorSetLayoutBinding texBinding{};
	texBinding.binding = 1;
	texBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	texBinding.descriptorCount = MAX_TEXTURES;
	texBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	texBinding.pImmutableSamplers = nullptr;

	VkDescriptorBindingFlags bindingFlags[] = {
		0,
		VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
		VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
		VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
	};
	VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.pNext = nullptr,
		.bindingCount = 1,
		.pBindingFlags = bindingFlags,
	};

	VkDescriptorSetLayoutCreateInfo layoutInfo1{};
	layoutInfo1.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo1.pNext = &flagsInfo;
	layoutInfo1.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
	layoutInfo1.bindingCount = 1;
	layoutInfo1.pBindings = &texBinding;

	if (vkCreateDescriptorSetLayout(
		m_pDevice->GetLogicalDevice(),
		&layoutInfo1,
		nullptr,
		&m_DescriptorSetLayout[1]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create bindless texture descriptor set layout!");
	}
}

void gp2::Pipeline::CreateGraphicsPipeline(const std::string& vertShaderPath, const std::string& fragShaderPath)
{
	Shader vertShader(m_pDevice, vertShaderPath);
	Shader fragShader(m_pDevice, fragShaderPath);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShader.GetShaderModule();
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShader.GetShaderModule();
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_pSwapChain->GetSwapChainExtent().width;
	viewport.height = (float)m_pSwapChain->GetSwapChainExtent().height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = m_pSwapChain->GetSwapChainExtent();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};

	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


	std::vector<VkDynamicState> dynamicStates =
	{
	   VK_DYNAMIC_STATE_VIEWPORT,
	   VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = VK_FALSE;
	depthStencil.front = {}; // Optional
	depthStencil.back = {}; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPushConstantRange pcRange{};
	pcRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pcRange.offset = 0;
	pcRange.size = sizeof(uint32_t);

	VkDescriptorSetLayout setLayouts[] = {
		m_DescriptorSetLayout[0],
		m_DescriptorSetLayout[1]   
	};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2;				
	pipelineLayoutInfo.pSetLayouts = setLayouts;	
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pcRange;

	if (vkCreatePipelineLayout(
		m_pDevice->GetLogicalDevice(),
		&pipelineLayoutInfo,
		nullptr,
		&m_PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}

	VkPipelineRenderingCreateInfo renderingInfo{};
	renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	renderingInfo.colorAttachmentCount = 1;
	//renderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_pSwapChain->GetSwapChainImages().size());
	renderingInfo.pColorAttachmentFormats = &m_pSwapChain->GetImageFormat();
	renderingInfo.depthAttachmentFormat = m_pSwapChain->GetDepthImage()->GetFormat();


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	//pipelineInfo.layout = m_Pipeline.GetPipelineLayout();
	pipelineInfo.layout = m_PipelineLayout;
	//if (m_pRenderPass != nullptr)
	//{
	//	pipelineInfo.renderPass = m_pRenderPass->GetRenderPass();
	//}
	//else
	//{
		pipelineInfo.pNext = &renderingInfo;
		pipelineInfo.renderPass = VK_NULL_HANDLE;
	//}
	//pipelineInfo.renderPass = m_pRenderPass->GetRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	
	if (vkCreateGraphicsPipelines(m_pDevice->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) 
	{
	   throw std::runtime_error("failed to create graphics pipeline!");
	}
}