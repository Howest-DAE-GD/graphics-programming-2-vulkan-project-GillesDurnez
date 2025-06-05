#include "LightPass.h"

#include <array>

gp2::LightPass::LightPass(Device* pDevice, CommandPool* pCommandPool, SwapChain* pSwapChain, BaseRenderPass* pGBuffer, DepthPrePass* pDepthPrePass, VkSampler sampler)
    : m_pDevice(pDevice)
    , m_pCommandPool(pCommandPool)
    , m_pSwapChain(pSwapChain)
    , m_TextureSampler(sampler)
	, m_pBaseRenderPass(pGBuffer)
	, m_pDepthPrePass(pDepthPrePass)
    , m_Pipeline({ m_pDevice, m_pSwapChain, &m_DescriptorPool, CreatePipeLineConfig(pDepthPrePass->GetDepthImage()) , "shaders/lightPass_vert.spv", "shaders/lightPass_frag.spv" })
{
    m_DescriptorPool.SetDescriptorSetPool(CreateDescriptorPool());
}

gp2::LightPass::~LightPass() = default;

void gp2::LightPass::RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene, Image* depthImage, Image* targetImage) const
{
    m_pDevice->GetDebugger().BeginLabel(commandBuffer, "Light Pass", { 0,1,1,1 });

    //depthImage->TransitionImageLayout(commandBuffer, depthImage->GetFormat(), depthImage->GetImageLayout(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT);
    targetImage->TransitionImageLayout(commandBuffer, m_pSwapChain->GetImageFormat(), m_pSwapChain->GetImages()[imageIndex].GetImageLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);
	GBuffer* m_pGBuffer = m_pBaseRenderPass->GetGBuffer();

    m_pGBuffer->diffuse.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    m_pGBuffer->normal.TransitionImageLayout(commandBuffer, m_pGBuffer->normal.GetFormat(), m_pGBuffer->normal.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);
    m_pGBuffer->metalnessAndRoughness.TransitionImageLayout(commandBuffer, m_pGBuffer->metalnessAndRoughness.GetFormat(), m_pGBuffer->metalnessAndRoughness.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_SHADER_READ_BIT, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT);

    m_pDepthPrePass->GetDepthImage()->TransitionImageLayout(
        commandBuffer,
        m_pDepthPrePass->GetDepthImage()->GetFormat(),
        m_pDepthPrePass->GetDepthImage()->GetImageLayout(),
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_ACCESS_2_SHADER_READ_BIT,
        VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        VK_IMAGE_ASPECT_DEPTH_BIT
    );
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
        std::vector<VkDescriptorSet> dSets = { m_GBufferDescriptorSet, m_UBODescriptorSets[imageIndex] };



        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, dSets.size(), dSets.data(), 0, nullptr);
        vkCmdDraw(commandBuffer, 3, 1, 0, 0);
    }
    vkCmdEndRendering(commandBuffer);

	targetImage->TransitionImageLayout(commandBuffer, m_pSwapChain->GetImageFormat(), targetImage->GetImageLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE);

    //m_pGBuffer->diffuse.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_2_NONE);
    //m_pGBuffer->normal.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_2_NONE);
    //m_pGBuffer->metalnessAndRoughness.TransitionImageLayout(commandBuffer, m_pGBuffer->diffuse.GetFormat(), m_pGBuffer->diffuse.GetImageLayout(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_2_SHADER_READ_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT, VK_PIPELINE_STAGE_2_NONE);

    m_pDevice->GetDebugger().EndLabel(commandBuffer);
}

void gp2::LightPass::Update(Camera* pCamera, uint32_t currentImage) const
{

}

void gp2::LightPass::RebindGbufferDescriptors()
{
    std::array<VkWriteDescriptorSet, 4> writes{};
    std::array<VkDescriptorImageInfo, 4> imageInfos{};

    GBuffer* m_pGBuffer = m_pBaseRenderPass->GetGBuffer();

    imageInfos[0] = { m_TextureSampler, m_pGBuffer->diffuse.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    imageInfos[1] = { m_TextureSampler, m_pGBuffer->normal.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
    imageInfos[2] = { m_TextureSampler, m_pGBuffer->metalnessAndRoughness.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	imageInfos[3] = { m_TextureSampler, m_pDepthPrePass->GetDepthImage()->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

    for (int i = 0; i < 4; ++i)
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

    //-- Depth Image ---------------------------------------------------------------------
    gBuffer.bindings.push_back(VkDescriptorSetLayoutBinding{
    .binding = 3,
    .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
    .descriptorCount = 1,
    .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
    .pImmutableSamplers = nullptr
        });

    gBuffer.info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    gBuffer.info.bindingCount = static_cast<uint32_t>(gBuffer.bindings.size());
    gBuffer.info.pBindings = gBuffer.bindings.data();
    layouts.push_back(std::move(gBuffer));

    DescriptorPool::DescriptorSetLayoutData ubo{};
    ubo.bindings = { VkDescriptorSetLayoutBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr } };

    ubo.info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ubo.info.bindingCount = static_cast<uint32_t>(ubo.bindings.size());
    ubo.info.pBindings = ubo.bindings.data();
    layouts.push_back(std::move(ubo));


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
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[0].descriptorCount = 4;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    poolInfo.maxSets = SwapChain::MAX_FRAMES_IN_FLIGHT + 1;
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

void gp2::LightPass::CreateDescriptorSets(Scene* /*pScene*/)
{
    VkDescriptorSetLayout gbLayout = m_DescriptorPool.GetDescriptorSetLayout(0);
    VkDescriptorSetAllocateInfo allocGB{};
    allocGB.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocGB.descriptorPool = m_DescriptorPool.GetDescriptorPool();
    allocGB.descriptorSetCount = 1;
    allocGB.pSetLayouts = &gbLayout;

    if (vkAllocateDescriptorSets(
        m_pDevice->GetLogicalDevice(),
        &allocGB,
        &m_GBufferDescriptorSet) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate G-Buffer descriptor set!");
    }

    std::array<VkWriteDescriptorSet, 4> writes{};
    std::array<VkDescriptorImageInfo, 4> imageInfos{};

    GBuffer* m_pGBuffer = m_pBaseRenderPass->GetGBuffer();

	imageInfos[0] = { m_TextureSampler, m_pGBuffer->diffuse.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	imageInfos[1] = { m_TextureSampler, m_pGBuffer->normal.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	imageInfos[2] = { m_TextureSampler, m_pGBuffer->metalnessAndRoughness.GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
	imageInfos[3] = { m_TextureSampler, m_pDepthPrePass->GetDepthImage()->GetImageView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };

    for (int i = 0; i < 4; ++i)
    {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = m_GBufferDescriptorSet;
        writes[i].dstBinding = (uint32_t)i;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[i].descriptorCount = 1;
        writes[i].pImageInfo = &imageInfos[i];
    }
    vkUpdateDescriptorSets(m_pDevice->GetLogicalDevice(),
        static_cast<uint32_t>(writes.size()),
        writes.data(),
        0, nullptr);

    VkDescriptorSetLayout uboLayout = m_DescriptorPool.GetDescriptorSetLayout(1);
    m_UBODescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    std::vector<VkDescriptorSetLayout> uboLayouts(SwapChain::MAX_FRAMES_IN_FLIGHT, uboLayout);
    VkDescriptorSetAllocateInfo allocUBO{};
    allocUBO.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocUBO.descriptorPool = m_DescriptorPool.GetDescriptorPool();
    allocUBO.descriptorSetCount = static_cast<uint32_t>(uboLayouts.size());
    allocUBO.pSetLayouts = uboLayouts.data();

    if (vkAllocateDescriptorSets(
        m_pDevice->GetLogicalDevice(),
        &allocUBO,
        m_UBODescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate UBO descriptor sets!");
    }

    for (uint32_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_pBaseRenderPass->GetUBOs()[i].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet write{};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = m_UBODescriptorSets[i];
        write.dstBinding = 0; 
        write.dstArrayElement = 0;
        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write.descriptorCount = 1;
        write.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(
            m_pDevice->GetLogicalDevice(),
            1, &write,
            0, nullptr);
    }
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
