#include "LightPass.h"

#include <array>

gp2::LightPass::LightPass(Device* pDevice, CommandPool* pCommandPool, SwapChain* pSwapChain, GBuffer* pGBuffer,
	Image* pDepthImage, VkSampler sampler)
    : m_pDevice(pDevice)
    , m_pCommandPool(pCommandPool)
    , m_pSwapChain(pSwapChain)
    , m_TextureSampler(sampler)
	, m_pGBuffer(pGBuffer)
	, m_pDepthImage(pDepthImage)
    , m_Pipeline({ m_pDevice, m_pSwapChain, &m_DescriptorPool, CreatePipeLineConfig(pDepthImage) , "shaders/lightPass_vert.spv", "shaders/lightPass_frag.spv" })
{
    m_DescriptorPool.SetDescriptorSetPool(CreateDescriptorPool());
}

gp2::LightPass::~LightPass() = default;

void gp2::LightPass::RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene, Image* depthImage, Image* targetImage) const
{

    //depthImage->TransitionImageLayout(commandBuffer, depthImage->GetFormat(), depthImage->GetImageLayout(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT);
    targetImage->TransitionImageLayout(commandBuffer, m_pSwapChain->GetImageFormat(), m_pSwapChain->GetImages()[imageIndex].GetImageLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    m_pGBuffer->diffuse.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    m_pGBuffer->normal.TransitionImageLayout(commandBuffer, m_pGBuffer->normal.GetFormat(), m_pGBuffer->normal.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    m_pGBuffer->metalnessAndRoughness.TransitionImageLayout(commandBuffer, m_pGBuffer->metalnessAndRoughness.GetFormat(), m_pGBuffer->metalnessAndRoughness.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

    //m_pGBuffer.diffuse.TransitionImageLayout(commandBuffer, m_pGBuffer.diffuse.GetFormat(), m_pGBuffer.diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, )

    std::vector<VkRenderingAttachmentInfo> colorAttachments{};

    VkRenderingAttachmentInfo colorAttachment = {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = targetImage->GetImageView();
    colorAttachment.imageLayout = targetImage->GetImageLayout();
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    colorAttachments.push_back(colorAttachment);

    VkRenderingInfo renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea = VkRect2D{ { 0, 0 }, m_pSwapChain->GetSwapChainExtent() };
    renderInfo.layerCount = 1;

    renderInfo.colorAttachmentCount = colorAttachments.size();
    renderInfo.pColorAttachments = colorAttachments.data();
    //renderInfo.pDepthAttachment = &depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    //std::array<VkWriteDescriptorSet, 3> writes{};
    //std::array<VkDescriptorImageInfo, 3> imageInfos{};

    //imageInfos[0] = { m_TextureSampler, m_pGBuffer->diffuse.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    //imageInfos[1] = { m_TextureSampler, m_pGBuffer->normal.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    //imageInfos[2] = { m_TextureSampler, m_pGBuffer->metalnessAndRoughness.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

    //for (int i = 0; i < 3; ++i) 
    //{
    //    writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    //    writes[i].dstSet = m_GBufferDescriptorSet;
    //    writes[i].dstBinding = i;
    //    writes[i].dstArrayElement = 0;
    //    writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    //    writes[i].descriptorCount = 1;
    //    writes[i].pImageInfo = &imageInfos[i];
    //}

    //vkUpdateDescriptorSets(m_pDevice->GetLogicalDevice(), writes.size(), writes.data(), 0, nullptr);

    //m_pDevice->GetDebugger().SetDebugName(commandBuffer, "Lightpass",)
    vkCmdBeginRendering(commandBuffer, &renderInfo);
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetGraphicsPipeline());

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_pSwapChain->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(m_pSwapChain->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_pSwapChain->GetSwapChainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);


        VkDeviceSize offsets[] = { 0 };
        std::vector<VkDescriptorSet> dSets = { m_GBufferDescriptorSet };



        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 1, dSets.data(), 0, nullptr);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }
    vkCmdEndRendering(commandBuffer);

	targetImage->TransitionImageLayout(commandBuffer, m_pSwapChain->GetImageFormat(), targetImage->GetImageLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE);

    //m_pGBuffer->diffuse.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_2_NONE);
    //m_pGBuffer->normal.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_2_NONE);
    //m_pGBuffer->metalnessAndRoughness.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_2_NONE);

}

void gp2::LightPass::Update(Camera* pCamera, uint32_t currentImage) const
{

}

std::vector<VkDescriptorSetLayout> gp2::LightPass::CreateDescriptorSetLayout() const
{
    std::vector<DescriptorPool::DescriptorSetLayoutData> layouts;

    // --- G Buffer -------------------------------------------------------------
    // --- --- Diffuse ----------------------------------------------------------
    DescriptorPool::DescriptorSetLayoutData gBuffer{};
    gBuffer.bindings.push_back(VkDescriptorSetLayoutBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    });

    // --- --- Normal map ----------------------------------------------------------
    gBuffer.bindings.push_back(VkDescriptorSetLayoutBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    });

    // --- --- Roughness & Metal ----------------------------------------------------------
    gBuffer.bindings.push_back(VkDescriptorSetLayoutBinding{
        .binding = 2,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    });

    gBuffer.info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    gBuffer.info.bindingCount = static_cast<uint32_t>(gBuffer.bindings.size());
    gBuffer.info.pBindings = gBuffer.bindings.data();
    layouts.push_back(std::move(gBuffer));

    // --- Create the Layouts ---------------------------------------------------
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    descriptorSetLayouts.resize(layouts.size());
    for (int index{}; index < layouts.size(); ++index)
    {
        auto layoutInfo = layouts[index];
        if (vkCreateDescriptorSetLayout(
            m_pDevice->GetLogicalDevice(),
            &layoutInfo.info,
            nullptr,
            &descriptorSetLayouts[index]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor set layout" + index);
        }
    }

    return descriptorSetLayouts;
}

VkDescriptorPool gp2::LightPass::CreateDescriptorPool() const
{
    std::array<VkDescriptorPoolSize, 1> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 3;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    poolInfo.maxSets = 1;
    VkDescriptorPool pool;
    if (vkCreateDescriptorPool(
        m_pDevice->GetLogicalDevice(),
        &poolInfo,
        nullptr,
        &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }

    return pool;
}

void gp2::LightPass::CreateDescriptorSets(Scene* pScene)
{
   
    VkDescriptorSetLayout texLayout = m_DescriptorPool.GetDescriptorSetLayout(0);
    uint32_t capacity = 1;

    VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
    countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
    countInfo.descriptorSetCount = 1;
    countInfo.pDescriptorCounts = &capacity;

    VkDescriptorSetAllocateInfo allocTex{};
    allocTex.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocTex.pNext = &countInfo;
    allocTex.descriptorPool = m_DescriptorPool.GetDescriptorPool();
    allocTex.descriptorSetCount = 1;
    allocTex.pSetLayouts = &texLayout;

    auto res = vkAllocateDescriptorSets(
        m_pDevice->GetLogicalDevice(),
        &allocTex,
        &m_GBufferDescriptorSet);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate bindless descriptor set!");
    }

    std::array<VkWriteDescriptorSet, 3> writes{};
	std::array<VkDescriptorImageInfo, 3> imageInfos{};

	imageInfos[0] = { m_TextureSampler, m_pGBuffer->diffuse.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	imageInfos[1] = { m_TextureSampler, m_pGBuffer->normal.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	imageInfos[2] = { m_TextureSampler, m_pGBuffer->metalnessAndRoughness.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

	for (int i = 0; i < 3; ++i) 
	{
	    writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	    writes[i].dstSet = m_GBufferDescriptorSet;
	    writes[i].dstBinding = i;
	    writes[i].dstArrayElement = 0;
	    writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	    writes[i].descriptorCount = 1;
	    writes[i].pImageInfo = &imageInfos[i];
	}

	vkUpdateDescriptorSets(m_pDevice->GetLogicalDevice(), writes.size(), writes.data(), 0, nullptr);
}

gp2::Pipeline::PipelineConfig& gp2::LightPass::CreatePipeLineConfig(Image* pDepthImage)
{
    //auto commandBuffer = m_pCommandPool->BeginSingleTimeCommands();

    ////m_pGBuffer->diffuse.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    ////m_pGBuffer->normal.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    ////m_pGBuffer->metalnessAndRoughness.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

    //m_pCommandPool->EndSingleTimeCommands(commandBuffer);

    // Depth
    m_PipelineConfig.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_PipelineConfig.depthStencil.depthTestEnable = VK_FALSE;
    m_PipelineConfig.depthStencil.depthWriteEnable = VK_FALSE;
    m_PipelineConfig.depthStencil.depthCompareOp = VK_COMPARE_OP_NEVER;
    m_PipelineConfig.depthStencil.depthBoundsTestEnable = VK_FALSE;
    m_PipelineConfig.depthStencil.minDepthBounds = 0.0f; // Optional
    m_PipelineConfig.depthStencil.maxDepthBounds = 1.0f; // Optional
    m_PipelineConfig.depthStencil.stencilTestEnable = VK_FALSE;
    m_PipelineConfig.depthStencil.front = {}; // Optional
    m_PipelineConfig.depthStencil.back = {}; // Optional

    m_PipelineConfig.colorAttatchmentFormats = std::vector<VkFormat>{
		m_pSwapChain->GetImageFormat()
    };

    // 
    m_PipelineConfig.renderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    m_PipelineConfig.renderInfo.colorAttachmentCount = m_PipelineConfig.colorAttatchmentFormats.size();
    //renderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_pSwapChain->GetSwapChainImages().size());
    m_PipelineConfig.renderInfo.pColorAttachmentFormats = m_PipelineConfig.colorAttatchmentFormats.data();
    m_PipelineConfig.renderInfo.depthAttachmentFormat = pDepthImage->GetFormat();

    // Depth Image
    m_PipelineConfig.depthImage = pDepthImage;

    return m_PipelineConfig;
}
