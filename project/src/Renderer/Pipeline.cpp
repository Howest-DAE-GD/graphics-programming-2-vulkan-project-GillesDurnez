#include "Pipeline.h"

#include <array>
#include <stdexcept>

#include "Resources/Model.h"
#include "Resources/Shader.h"

gp2::Pipeline::Pipeline(Device* device, SwapChain* swapChain, DescriptorPool* pDescriptorPool, const PipelineConfig& pipelineConfig, const std::string& vertShaderPath, const std::string& fragShaderPath)
	: m_pDevice(device)
	, m_pSwapChain(swapChain)
	, m_pDescriptorPool(pDescriptorPool)
{
	CreateGraphicsPipeline(pipelineConfig, vertShaderPath, fragShaderPath);
}

gp2::Pipeline::~Pipeline()
{
	vkDestroyPipeline(m_pDevice->GetLogicalDevice(), m_GraphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_pDevice->GetLogicalDevice(), m_PipelineLayout, nullptr);
}

gp2::Pipeline::Pipeline(Pipeline&& other)
{
	m_pDevice = other.m_pDevice;
	m_pDescriptorPool = other.m_pDescriptorPool;
	m_pSwapChain = other.m_pSwapChain;
	m_pDescriptorPool = other.m_pDescriptorPool;

	m_PipelineLayout = std::move(other.m_PipelineLayout);
	m_GraphicsPipeline = std::move(other.m_GraphicsPipeline);

	other.m_PipelineLayout = VK_NULL_HANDLE;
	other.m_GraphicsPipeline = VK_NULL_HANDLE;
}

gp2::Pipeline& gp2::Pipeline::operator=(Pipeline&& other)
{
	m_pDevice = other.m_pDevice;
	m_pDescriptorPool = other.m_pDescriptorPool;
	m_pSwapChain = other.m_pSwapChain;
	m_pDescriptorPool = other.m_pDescriptorPool;

	m_PipelineLayout = std::move(other.m_PipelineLayout);
	m_GraphicsPipeline = std::move(other.m_GraphicsPipeline);

	other.m_PipelineLayout = VK_NULL_HANDLE;
	other.m_GraphicsPipeline = VK_NULL_HANDLE;

	return *this;
}

void gp2::Pipeline::CreateGraphicsPipeline(const PipelineConfig& pipelineConfig, const std::string& vertShaderPath, const std::string& fragShaderPath)
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};

	Shader vertShader(m_pDevice, vertShaderPath);
	Shader fragShader{};

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShader.GetShaderModule();
	vertShaderStageInfo.pName = "main";
	shaderStages.push_back(vertShaderStageInfo);

	if (!fragShaderPath.empty())
	{
		fragShader = Shader{ m_pDevice, fragShaderPath };
		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShader.GetShaderModule();
		fragShaderStageInfo.pName = "main";
		shaderStages.push_back(fragShaderStageInfo);
	}


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

	std::vector<VkPipelineColorBlendAttachmentState> cStates{};

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	for (int index{}; index < pipelineConfig.renderInfo.colorAttachmentCount; ++index)
	{
		cStates.push_back(colorBlendAttachment);
	}

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = cStates.size();
	colorBlending.pAttachments = cStates.data();
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = m_pDescriptorPool->GetDescriptorSetLayouts().size();				
	pipelineLayoutInfo.pSetLayouts = m_pDescriptorPool->GetDescriptorSetLayouts().data();
	pipelineLayoutInfo.pushConstantRangeCount = pipelineConfig.pushConstants.size();
	pipelineLayoutInfo.pPushConstantRanges = pipelineConfig.pushConstants.data();

	if (vkCreatePipelineLayout(
		m_pDevice->GetLogicalDevice(),
		&pipelineLayoutInfo,
		nullptr,
		&m_PipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create pipeline layout!");
	}


	//VkPipelineRenderingCreateInfo renderingInfo{};
	//renderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	//renderingInfo.colorAttachmentCount = 1;
	////renderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_pSwapChain->GetSwapChainImages().size());
	//renderingInfo.pColorAttachmentFormats = &m_pSwapChain->GetImageFormat();
	//renderingInfo.depthAttachmentFormat = pipelineConfig.depthImage->GetFormat();


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = shaderStages.size();
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &pipelineConfig.depthStencil; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_PipelineLayout;

	pipelineInfo.pNext = &pipelineConfig.renderInfo;
	pipelineInfo.renderPass = VK_NULL_HANDLE;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	
	if (vkCreateGraphicsPipelines(m_pDevice->GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_GraphicsPipeline) != VK_SUCCESS) 
	{
	   throw std::runtime_error("failed to create graphics pipeline!");
	}
}
