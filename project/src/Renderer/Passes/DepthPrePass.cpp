#include "DepthPrePass.h"

#include "BaseRenderPass.h"

gp2::DepthPrePass::DepthPrePass(Device* pDevice, CommandPool* pCommandPool, SwapChain* pSwapChain)
	: m_pDevice(pDevice)
	, m_pCommandPool(pCommandPool)
	, m_pSwapChain(pSwapChain)
{
	CreateUniformBuffers();
	m_DescriptorPool.SetDescriptorSetPool(CreateDescriptorPool());
}

gp2::DepthPrePass::~DepthPrePass()
{
	delete m_pDepthImage;
	m_pDepthImage = nullptr;
}

void gp2::DepthPrePass::CreateDescriptorSets(Scene* pScene)
{
    m_UBODescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    {
        std::vector<VkDescriptorSetLayout> uboLayouts(
            SwapChain::MAX_FRAMES_IN_FLIGHT,
            m_DescriptorPool.GetDescriptorSetLayout(0)
        );

        VkDescriptorSetAllocateInfo allocUBO{};
        allocUBO.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocUBO.descriptorPool = m_DescriptorPool.GetDescriptorPool();
        allocUBO.descriptorSetCount = uint32_t(uboLayouts.size());
        allocUBO.pSetLayouts = uboLayouts.data();

        if (vkAllocateDescriptorSets(
            m_pDevice->GetLogicalDevice(),
            &allocUBO,
            m_UBODescriptorSets.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate UBO descriptor sets!");
        }

        // write each UBO
        for (uint32_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = m_UniformBuffers[i].GetBuffer();
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
                0, nullptr
            );
        }
    }
}

void gp2::DepthPrePass::RecordCommandBuffer(VkCommandBuffer& commandBuffer, uint32_t imageIndex, Scene* pScene) const
{
	m_pDevice->GetDebugger().BeginLabel(commandBuffer, "Depth Pre-Pass", {0,0,0,1});
    m_pDepthImage->TransitionImageLayout(commandBuffer, m_pDepthImage->GetFormat(), m_pDepthImage->GetImageLayout(), VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
    m_pSwapChain->GetImages()[imageIndex].TransitionImageLayout(commandBuffer, m_pSwapChain->GetImageFormat(), m_pSwapChain->GetImages()[imageIndex].GetImageLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkRenderingAttachmentInfo depthAttachment = {};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = m_pDepthImage->GetImageView();
    depthAttachment.imageLayout = m_pDepthImage->GetImageLayout();
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

    VkRenderingInfo renderInfo{};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.renderArea = VkRect2D{ { 0, 0 }, m_pSwapChain->GetSwapChainExtent() };
    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 0;
    renderInfo.pColorAttachments = nullptr;
    renderInfo.pDepthAttachment = &depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering(commandBuffer, &renderInfo);
    {
        for (auto model : pScene->GetModels())
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


            VkBuffer vertexBuffers[] = { model->GetVertexBuffer()->GetBuffer() };
            VkDeviceSize offsets[] = { 0 };
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
            vkCmdBindIndexBuffer(commandBuffer, model->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

            std::vector<VkDescriptorSet> dSets = { m_UBODescriptorSets[imageIndex] };

            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 1, dSets.data(), 0, nullptr);
            vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->GetIndices().size()), 1, 0, 0, 0);
        }
    }
    vkCmdEndRendering(commandBuffer);

    m_pSwapChain->GetImages()[imageIndex].TransitionImageLayout(commandBuffer, m_pSwapChain->GetImageFormat(), m_pSwapChain->GetImages()[imageIndex].GetImageLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE);
	m_pDevice->GetDebugger().EndLabel(commandBuffer);
}

void gp2::DepthPrePass::Update(Camera* pCamera, uint32_t currentImage) const
{
    UniformBufferObject ubo{};
    ubo.model = glm::mat4(1.0f);
    ubo.view = pCamera->viewMatrix;
    ubo.proj = pCamera->projectionMatrix;
    m_UniformBuffers[currentImage].CopyMemory(&ubo, sizeof(ubo), 0);
}

std::vector<VkDescriptorSetLayout> gp2::DepthPrePass::CreateDescriptorSetLayout() const
{
    std::vector<DescriptorPool::DescriptorSetLayoutData> layouts;

    // --- UBO set ------------------------------------------------------------
    DescriptorPool::DescriptorSetLayoutData ubo{};
    ubo.bindings = { VkDescriptorSetLayoutBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr } };

    ubo.info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ubo.info.bindingCount = static_cast<uint32_t>(ubo.bindings.size());
    ubo.info.pBindings = ubo.bindings.data();
    layouts.push_back(std::move(ubo));

    // --- Texture set --------------------------------------------------------
    constexpr uint32_t MAX_TEXTURES = 512;

    DescriptorPool::DescriptorSetLayoutData tex{};
    tex.bindings = { VkDescriptorSetLayoutBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = MAX_TEXTURES,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr } };

    VkDescriptorBindingFlags flags[] = {
        0,
        VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
        VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
        VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT };

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .pNext = nullptr,
        .bindingCount = 1,
        .pBindingFlags = flags };

    tex.info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    tex.info.pNext = &flagsInfo;
    tex.info.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
    tex.info.bindingCount = static_cast<uint32_t>(tex.bindings.size());
    tex.info.pBindings = tex.bindings.data();
    layouts.push_back(std::move(tex));

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

VkDescriptorPool gp2::DepthPrePass::CreateDescriptorPool() const
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = DescriptorPool::MAX_POOL_RESERVE;

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

void gp2::DepthPrePass::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_UniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = bufferSize;
        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        m_UniformBuffers[i] = Buffer{ m_pDevice, m_pCommandPool, bufferInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true };
    }
}

gp2::Pipeline::PipelineConfig& gp2::DepthPrePass::CreatePipeLineConfig()
{
    // DepthStencil
    m_PipelineConfig.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_PipelineConfig.depthStencil.depthTestEnable = VK_TRUE;
    m_PipelineConfig.depthStencil.depthWriteEnable = VK_TRUE;
    m_PipelineConfig.depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    m_PipelineConfig.depthStencil.depthBoundsTestEnable = VK_FALSE;
    m_PipelineConfig.depthStencil.minDepthBounds = 0.0f; // Optional
    m_PipelineConfig.depthStencil.maxDepthBounds = 1.0f; // Optional
    m_PipelineConfig.depthStencil.stencilTestEnable = VK_FALSE;
    m_PipelineConfig.depthStencil.front = {}; // Optional
    m_PipelineConfig.depthStencil.back = {}; // Optional

    // Render Info
    m_PipelineConfig.renderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    m_PipelineConfig.renderInfo.depthAttachmentFormat = m_pDepthImage->GetFormat();

    // Push Constants

    // Depth Image
    m_PipelineConfig.depthImage = m_pDepthImage;

    return m_PipelineConfig;
}

gp2::Image* gp2::DepthPrePass::ReCreateDepthResource()
{
    VkFormat depthFormat = m_pDevice->FindDepthFormat();

	gp2::Image::ImageCreateInfo depthImageCreateInfo{};

	depthImageCreateInfo.width = m_pSwapChain->GetSwapChainExtent().width;
	depthImageCreateInfo.height = m_pSwapChain->GetSwapChainExtent().height;
	depthImageCreateInfo.format = depthFormat;
	depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	depthImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	depthImageCreateInfo.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthImageCreateInfo.properties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

    if (m_pDepthImage != nullptr) 
    {
        delete m_pDepthImage;
		m_pDepthImage = nullptr;
    }

	m_pDepthImage = new Image{ m_pDevice, m_pCommandPool, depthImageCreateInfo };

	VkCommandBuffer commandBuffer = m_pCommandPool->BeginSingleTimeCommands();

    m_pDepthImage->TransitionImageLayout(commandBuffer, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    m_pCommandPool->EndSingleTimeCommands(commandBuffer);

    vmaSetAllocationName(m_pDevice->GetAllocator(), m_pDepthImage->GetImageAllocation(), "Swapchain depth image Buffer");
    return m_pDepthImage;
}

gp2::Image* gp2::DepthPrePass::CreateDepthResource()
{
    m_pDepthImage = nullptr;
    return ReCreateDepthResource();
}
