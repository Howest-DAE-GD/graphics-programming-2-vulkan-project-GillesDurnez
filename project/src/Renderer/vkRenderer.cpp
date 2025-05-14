#include "vkRenderer.h"

#include <chrono>
#include <stdexcept>

gp2::VkRenderer::VkRenderer()
{
    m_Scene.AddTexture(new Texture{ &m_Device, &m_CommandPool, "textures/viking_room.png" });
    m_Scene.AddModel(new Model{ &m_Device, &m_CommandPool, "models/viking_room.obj" });

    //CreateFrameBuffers();
    CreateTextureSampler();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateSyncObjects();
}

gp2::VkRenderer::~VkRenderer()
{
    vkDeviceWaitIdle(m_Device.GetLogicalDevice());
    //CleanupSwapChain();

    vkDestroySampler(m_Device.GetLogicalDevice(), m_TextureSampler, nullptr);

    //for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    //{
    //    vkDestroyBuffer(m_Device.GetLogicalDevice(), m_UniformBuffers[i], nullptr);
    //    vkFreeMemory(m_Device.GetLogicalDevice(), m_UniformBuffersMemory[i], nullptr);
    //}

    vkDestroyDescriptorPool(m_Device.GetLogicalDevice(), m_DescriptorPool, nullptr);

    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        vkDestroySemaphore(m_Device.GetLogicalDevice(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device.GetLogicalDevice(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_Device.GetLogicalDevice(), m_InFlightFences[i], nullptr);
    }

}

void gp2::VkRenderer::RenderFrame()
{
    m_Camera.Update();

    vkWaitForFences(m_Device.GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(m_Device.GetLogicalDevice(), m_SwapChain.GetSwapChain(), UINT64_MAX, m_ImageAvailableSemaphores[m_CurrentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        RecreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    UpdateUniformBuffer(m_CurrentFrame);

    vkResetFences(m_Device.GetLogicalDevice(), 1, &m_InFlightFences[m_CurrentFrame]);

    vkResetCommandBuffer(m_CommandPool.GetCommandBuffers()[m_CurrentFrame], 0);
    RecordCommandBuffer(m_CommandPool.GetCommandBuffers()[m_CurrentFrame], imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_ImageAvailableSemaphores[m_CurrentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CommandPool.GetCommandBuffers()[m_CurrentFrame];

    VkSemaphore signalSemaphores[] = { m_RenderFinishedSemaphores[m_CurrentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_Device.GetGraphicsQueue(), 1, &submitInfo, m_InFlightFences[m_CurrentFrame]) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    VkSwapchainKHR swapChains[] = { m_SwapChain.GetSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr; // Optional

    result = vkQueuePresentKHR(m_Device.GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_FramebufferResized)
    {
        m_FramebufferResized = false;
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

//void gp2::VkRenderer::CreateFrameBuffers()
//{
//    m_SwapChainFramebuffers.resize(m_SwapChain.GetImageViews().size());
//
//    for (size_t i = 0; i < m_SwapChain.GetImageViews().size(); i++)
//    {
//        std::array<VkImageView, 2> attachments = { m_SwapChain.GetImageViews()[i], m_SwapChain.GetDepthImage()->GetImageView() };
//
//        VkFramebufferCreateInfo framebufferInfo{};
//        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        framebufferInfo.renderPass = VK_NULL_HANDLE;
//        //framebufferInfo.renderPass = m_RenderPass.GetRenderPass();
//        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//        framebufferInfo.pAttachments = attachments.data();
//        framebufferInfo.width = m_SwapChain.GetSwapChainExtent().width;
//        framebufferInfo.height = m_SwapChain.GetSwapChainExtent().height;
//        framebufferInfo.layers = 1;
//
//        if (vkCreateFramebuffer(m_Device.GetLogicalDevice(), &framebufferInfo, nullptr, &m_SwapChainFramebuffers[i]) != VK_SUCCESS)
//        {
//            throw std::runtime_error("Failed to create framebuffer!");
//        }
//    }
//}

void gp2::VkRenderer::CreateTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_Device.GetPhysicalDevice(), &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_Device.GetLogicalDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void gp2::VkRenderer::CreateUniformBuffers()
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

		m_UniformBuffers[i] = Buffer{ &m_Device, &m_CommandPool, bufferInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true };
    }
}

void gp2::VkRenderer::CreateDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(m_Device.GetLogicalDevice(), &poolInfo, nullptr, &m_DescriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void gp2::VkRenderer::CreateDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(SwapChain::MAX_FRAMES_IN_FLIGHT, m_Pipeline.GetDescriptorSetLayout());
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_DescriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_DescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(m_Device.GetLogicalDevice(), &allocInfo, m_DescriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_UniformBuffers[i].GetBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_Scene.GetTexture(0)->GetTextureImage()->GetImageView();
        imageInfo.sampler = m_TextureSampler;


        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_DescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_DescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(m_Device.GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }
}

void gp2::VkRenderer::CreateSyncObjects()
{
    m_ImageAvailableSemaphores.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    m_RenderFinishedSemaphores.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    m_InFlightFences.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(m_Device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_Device.GetLogicalDevice(), &semaphoreInfo, nullptr, &m_RenderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_Device.GetLogicalDevice(), &fenceInfo, nullptr, &m_InFlightFences[i]) != VK_SUCCESS)
        {

            throw std::runtime_error("failed to create synchronization objects for a frame!");
        }
    }
}

//void gp2::VkRenderer::CleanupSwapChain() const
//{
//    for (size_t i = 0; i < m_SwapChainFramebuffers.size(); i++)
//    {
//        vkDestroyFramebuffer(m_Device.GetLogicalDevice(), m_SwapChainFramebuffers[i], nullptr);
//    }
//}

void gp2::VkRenderer::RecreateSwapChain()
{
    m_SwapChain.RecreateSwapChain();
    //CleanupSwapChain();

    //CreateFrameBuffers();
    m_Camera.aspectRatio = m_SwapChain.GetSwapChainExtent().width / static_cast<float>(m_SwapChain.GetSwapChainExtent().height);
}

void gp2::VkRenderer::UpdateUniformBuffer(uint32_t currentImage) const
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.model = glm::mat4(1.0f);
    //ubo.view = m_Camera.viewMatrix;
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //ubo.proj = m_Camera.projectionMatrix;
	ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain.GetSwapChainExtent().width / (float)m_SwapChain.GetSwapChainExtent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;
	m_UniformBuffers[currentImage].CopyMemory(&ubo, sizeof(ubo), 0);
    //memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void gp2::VkRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

	m_SwapChain.GetDepthImage()->TransitionImageLayout(commandBuffer, m_SwapChain.GetDepthImage()->GetFormat(), m_SwapChain.GetDepthImage()->GetImageLayout(),VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE,VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT);
	m_SwapChain.GetImages()[imageIndex].TransitionImageLayout(commandBuffer, m_SwapChain.GetImageFormat(), m_SwapChain.GetImages()[imageIndex].GetImageLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

    VkRenderingAttachmentInfo colorAttachment = {};
	colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView = m_SwapChain.GetImages()[imageIndex].GetImageView();
	colorAttachment.imageLayout = m_SwapChain.GetImages()[imageIndex].GetImageLayout();
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

	VkRenderingAttachmentInfo depthAttachment = {};
	depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	depthAttachment.imageView = m_SwapChain.GetDepthImage()->GetImageView();
	depthAttachment.imageLayout = m_SwapChain.GetDepthImage()->GetImageLayout();
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

	VkRenderingInfo renderInfo{};
	renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderInfo.renderArea = VkRect2D{ { 0, 0 }, m_SwapChain.GetSwapChainExtent() };
	renderInfo.layerCount = 1;
	renderInfo.colorAttachmentCount = 1;
	renderInfo.pColorAttachments = &colorAttachment;
	renderInfo.pDepthAttachment = &depthAttachment;
	renderInfo.pStencilAttachment = nullptr;

    //VkCommandBuffer cmndBuffer =  m_CommandPool.BeginSingleTimeCommands();
    vkCmdBeginRendering(commandBuffer, &renderInfo);
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetGraphicsPipeline());
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_SwapChain.GetSwapChainExtent().width);
        viewport.height = static_cast<float>(m_SwapChain.GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_SwapChain.GetSwapChainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { m_Scene.GetModel(0)->GetVertexBuffer()->GetBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_Scene.GetModel(0)->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 1, &m_DescriptorSets[imageIndex], 0, nullptr);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Scene.GetModel(0)->GetIndices().size()), 1, 0, 0, 0);
    }
    vkCmdEndRendering(commandBuffer);
	//m_CommandPool.EndSingleTimeCommands(cmndBuffer);

    //VkCommandBuffer commandBuffer = m_CommandPool.BeginSingleTimeCommands();
    m_SwapChain.GetImages()[imageIndex].TransitionImageLayout(commandBuffer, m_SwapChain.GetImageFormat(), m_SwapChain.GetImages()[imageIndex].GetImageLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE);
    //m_CommandPool.EndSingleTimeCommands(commandBuffer);


    //VkRenderPassBeginInfo renderPassInfo{};
    //renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    //renderPassInfo.renderPass = VK_NULL_HANDLE;
    ////renderPassInfo.renderPass = m_RenderPass.GetRenderPass();
    //renderPassInfo.framebuffer = m_SwapChainFramebuffers[imageIndex];

    //renderPassInfo.renderArea.offset = { 0, 0 };
    //renderPassInfo.renderArea.extent = m_SwapChain.GetSwapChainExtent();

    //std::array<VkClearValue, 2> clearValues{};
    //clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
    //clearValues[1].depthStencil = { 1.0f, 0 };

    //renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    //renderPassInfo.pClearValues = clearValues.data();

    //vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    //{
    //    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetGraphicsPipeline());

    //    VkViewport viewport{};
    //    viewport.x = 0.0f;
    //    viewport.y = 0.0f;
    //    viewport.width = static_cast<float>(m_SwapChain.GetSwapChainExtent().width);
    //    viewport.height = static_cast<float>(m_SwapChain.GetSwapChainExtent().height);
    //    viewport.minDepth = 0.0f;
    //    viewport.maxDepth = 1.0f;
    //    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    //    VkRect2D scissor{};
    //    scissor.offset = { 0, 0 };
    //    scissor.extent = m_SwapChain.GetSwapChainExtent();
    //    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    //    VkBuffer vertexBuffers[] = { m_Scene.GetModel(0)->GetVertexBuffer()->GetBuffer() };
    //    VkDeviceSize offsets[] = { 0 };
    //    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    //    vkCmdBindIndexBuffer(commandBuffer, m_Scene.GetModel(0)->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    //    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 1, &m_DescriptorSets[m_CurrentFrame], 0, nullptr);
    //    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_Scene.GetModel(0)->GetIndices().size()), 1, 0, 0, 0);
    //}
    //vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
}
