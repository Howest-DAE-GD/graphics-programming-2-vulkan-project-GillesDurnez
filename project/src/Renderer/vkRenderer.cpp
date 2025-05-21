#include "vkRenderer.h"

#include <chrono>
#include <stdexcept>

gp2::VkRenderer::VkRenderer()
{
    m_Scene.LoadScene(&m_Device, &m_CommandPool, "../scenes/Sponza/Sponza.gltf");

    //CreateTextureSampler();
    //CreateUniformBuffers();
    //CreateDescriptorPool();
    //CreateDescriptorSets();
    m_BaseRenderPass.CreateDescriptorSets(&m_Scene);
    CreateSyncObjects();
}

gp2::VkRenderer::~VkRenderer()
{
    vkDeviceWaitIdle(m_Device.GetLogicalDevice());
    //CleanupSwapChain();

    vkDestroySampler(m_Device.GetLogicalDevice(), m_TextureSampler, nullptr);


    //vkDestroyDescriptorPool(m_Device.GetLogicalDevice(), m_DescriptorPool, nullptr);

    for (size_t i = 0; i < m_RenderFinishedSemaphores.size(); i++)
    {
        vkDestroySemaphore(m_Device.GetLogicalDevice(), m_RenderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_Device.GetLogicalDevice(), m_ImageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_Device.GetLogicalDevice(), m_InFlightFences[i], nullptr);
    }

}

//void gp2::VkRenderer::UpdateAllTextureDescriptors()
//{
//
//    uint32_t actualCount = static_cast<uint32_t>(m_Scene.GetTextures().size());
//    std::vector<VkWriteDescriptorSet> writes;
//    writes.reserve(actualCount);
//
//    for (uint32_t i = 0; i < actualCount; ++i) {
//        VkDescriptorImageInfo info{};
//        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//        info.imageView = m_Scene.GetTexture(i)->GetTextureImage()->GetImageView();
//        info.sampler = m_TextureSampler;
//
//        VkWriteDescriptorSet w{};
//        w.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//        w.dstSet = m_TextureDescriptorSet;
//        w.dstBinding = 1;
//        w.dstArrayElement = i;               // slot = texture index
//        w.descriptorCount = 1;
//        w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//        w.pImageInfo = &info;
//
//        writes.push_back(w);
//    }
//
//    vkUpdateDescriptorSets(
//        m_Device.GetLogicalDevice(),
//        static_cast<uint32_t>(writes.size()),
//        writes.data(),
//        0, nullptr
//    );
//}

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
    frameCount++;
}

VkSampler gp2::VkRenderer::CreateTextureSampler()
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
    return m_TextureSampler;
}

//void gp2::VkRenderer::CreateUniformBuffers()
//{
//    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
//
//    m_UniformBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
//
//    for (size_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++)
//    {
//        VkBufferCreateInfo bufferInfo{};
//        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//        bufferInfo.size = bufferSize;
//        bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
//        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//		m_UniformBuffers[i] = Buffer{ &m_Device, &m_CommandPool, bufferInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true };
//    }
//}

//std::vector<VkDescriptorSetLayoutCreateInfo> gp2::VkRenderer::CreateDescriptorSetLayout()
//{
//    //// DESCRIPTOR LAYOUTS
//    //std::vector<VkDescriptorSetLayoutCreateInfo> dsLayouts{};
//    //dsLayouts.resize(2);
//    //// UBO DescriptorSet
//    //VkDescriptorSetLayoutBinding uboBinding{};
//    //uboBinding.binding = 0;
//    //uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    //uboBinding.descriptorCount = 1;
//    //uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//    //uboBinding.pImmutableSamplers = nullptr;
//
//    //VkDescriptorSetLayoutCreateInfo layoutInfo0{};
//    //layoutInfo0.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    //layoutInfo0.bindingCount = 1;
//    //layoutInfo0.pBindings = &uboBinding;
//
//    //dsLayouts.push_back(layoutInfo0);
//
//    //// Create texture descriptor sets
//    //constexpr uint32_t MAX_TEXTURES = 512;
//    //VkDescriptorSetLayoutBinding texBinding{};
//    //texBinding.binding = 1;
//    //texBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    //texBinding.descriptorCount = MAX_TEXTURES;
//    //texBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//    //texBinding.pImmutableSamplers = nullptr;
//
//    //VkDescriptorBindingFlags bindingFlags[] = {
//    //    0,
//    //    VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
//    //    VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
//    //    VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT
//    //};
//    //VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo{
//    //    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
//    //    .pNext = nullptr,
//    //    .bindingCount = 1,
//    //    .pBindingFlags = bindingFlags,
//    //};
//
//    //VkDescriptorSetLayoutCreateInfo layoutInfo1{};
//    //layoutInfo1.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    //layoutInfo1.pNext = &flagsInfo;
//    //layoutInfo1.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
//    //layoutInfo1.bindingCount = 1;
//    //layoutInfo1.pBindings = &texBinding;
//
//    //dsLayouts.push_back(layoutInfo1);
//
//    //return dsLayouts;
//}
//
//VkDescriptorPoolCreateInfo gp2::VkRenderer::CreateDescriptorPool()
//{
//    // Create DescriptorPool
//
//    //std::array<VkDescriptorPoolSize, 2> poolSizes{};
//    //poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    //poolSizes[0].descriptorCount = static_cast<uint32_t>(SwapChain::MAX_FRAMES_IN_FLIGHT);
//
//    //poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    //poolSizes[1].descriptorCount = DescriptorPool::MAX_POOL_RESERVE;
//
//    //VkDescriptorPoolCreateInfo poolInfo{};
//    //poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//    //poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
//    //poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
//    //poolInfo.pPoolSizes = poolSizes.data();
//
//    //poolInfo.maxSets = SwapChain::MAX_FRAMES_IN_FLIGHT + 1;
//    //return poolInfo;
//
//    //if (vkCreateDescriptorPool(
//    //    m_Device.GetLogicalDevice(),
//    //    &poolInfo,
//    //    nullptr,
//    //    &m_DescriptorPool) != VK_SUCCESS)
//    //{
//    //    throw std::runtime_error("failed to create descriptor pool!");
//    //}   
//}

//void gp2::VkRenderer::CreateDescriptorSets()
//{
//    //m_UBODescriptorSets.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
//    //{
//    //    std::vector<VkDescriptorSetLayout> uboLayouts(
//    //        SwapChain::MAX_FRAMES_IN_FLIGHT,
//    //        m_Pipeline.GetDescriptorSetLayout(0)
//    //    );
//
//    //    VkDescriptorSetAllocateInfo allocUBO{};
//    //    allocUBO.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    //    allocUBO.descriptorPool = m_DescriptorPool;
//    //    allocUBO.descriptorSetCount = uint32_t(uboLayouts.size());
//    //    allocUBO.pSetLayouts = uboLayouts.data();
//
//    //    if (vkAllocateDescriptorSets(
//    //        m_Device.GetLogicalDevice(),
//    //        &allocUBO,
//    //        m_UBODescriptorSets.data()) != VK_SUCCESS)
//    //    {
//    //        throw std::runtime_error("failed to allocate UBO descriptor sets!");
//    //    }
//
//    //    // write each UBO
//    //    for (uint32_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i) {
//    //        VkDescriptorBufferInfo bufferInfo{};
//    //        bufferInfo.buffer = m_UniformBuffers[i].GetBuffer();
//    //        bufferInfo.offset = 0;
//    //        bufferInfo.range = sizeof(UniformBufferObject);
//
//    //        VkWriteDescriptorSet write{};
//    //        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    //        write.dstSet = m_UBODescriptorSets[i];
//    //        write.dstBinding = 0;  
//    //        write.dstArrayElement = 0;
//    //        write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    //        write.descriptorCount = 1;
//    //        write.pBufferInfo = &bufferInfo;
//
//    //        vkUpdateDescriptorSets(
//    //            m_Device.GetLogicalDevice(),
//    //            1, &write,
//    //            0, nullptr
//    //        );
//    //    }
//    //}
//
//    //constexpr uint32_t MAX_TEXTURES = 512;
//    //uint32_t capacity = MAX_TEXTURES;
//
//    //VkDescriptorSetLayout texLayout = m_Pipeline.GetDescriptorSetLayout(1);
//
//    //VkDescriptorSetVariableDescriptorCountAllocateInfo countInfo{};
//    //countInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO;
//    //countInfo.descriptorSetCount = 1;
//    //countInfo.pDescriptorCounts = &capacity;
//
//    //VkDescriptorSetAllocateInfo allocTex{};
//    //allocTex.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    //allocTex.pNext = &countInfo;
//    //allocTex.descriptorPool = m_DescriptorPool;
//    //allocTex.descriptorSetCount = 1;
//    //allocTex.pSetLayouts = &texLayout;
//
//    //VkDescriptorSet texSet;
//    //if (vkAllocateDescriptorSets(
//    //    m_Device.GetLogicalDevice(),
//    //    &allocTex,
//    //    &texSet) != VK_SUCCESS)
//    //{
//    //    throw std::runtime_error("failed to allocate bindless descriptor set!");
//    //}
//
//    //m_TextureDescriptorSet = texSet;  // store for later updates
//
//    //uint32_t actualCount = uint32_t(m_Scene.GetTextures().size());
//    //std::vector<VkDescriptorImageInfo> imageInfos(actualCount);
//
//    //for (uint32_t i = 0; i < actualCount; ++i) {
//    //    imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//    //    imageInfos[i].imageView = m_Scene.GetTexture(i)
//    //        ->GetTextureImage()
//    //        ->GetImageView();
//    //    imageInfos[i].sampler = m_TextureSampler;
//    //}
//
//    //VkWriteDescriptorSet imageWrite{};
//    //imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//    //imageWrite.dstSet = texSet;
//    //imageWrite.dstBinding = 1;               
//    //imageWrite.dstArrayElement = 0;
//    //imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    //imageWrite.descriptorCount = actualCount;
//    //imageWrite.pImageInfo = imageInfos.data();
//
//    //vkUpdateDescriptorSets(
//    //    m_Device.GetLogicalDevice(),
//    //    1, &imageWrite,
//    //    0, nullptr
//    //);
//}

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

void gp2::VkRenderer::RecreateSwapChain()
{
    m_SwapChain.RecreateSwapChain();
    //CleanupSwapChain();

    //CreateFrameBuffers();
    m_Camera.aspectRatio = m_SwapChain.GetSwapChainExtent().width / static_cast<float>(m_SwapChain.GetSwapChainExtent().height);
}

void gp2::VkRenderer::UpdateUniformBuffer(uint32_t currentImage)
{
    m_BaseRenderPass.Update(&m_Camera, currentImage);

 //   static auto startTime = std::chrono::high_resolution_clock::now();

 //   auto currentTime = std::chrono::high_resolution_clock::now();
 //   float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

 //   UniformBufferObject ubo{};
 //   //ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	//ubo.model = glm::mat4(1.0f);
 //   ubo.view = m_Camera.viewMatrix;
 //   //ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
 //   ubo.proj = m_Camera.projectionMatrix;
	////ubo.proj = glm::perspective(glm::radians(45.0f), m_SwapChain.GetSwapChainExtent().width / (float)m_SwapChain.GetSwapChainExtent().height, 0.1f, 10.0f);
 //   //ubo.proj[1][1] *= -1;
	//m_UniformBuffers[currentImage].CopyMemory(&ubo, sizeof(ubo), 0);
 //   //memcpy(m_UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
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

    {
        //VkRenderingAttachmentInfo depthAttachmentInfo{};
        //depthAttachmentInfo.imageView = m_DepthPrepassImage[imageIndex].GetImageView();
        //depthAttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        //depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        //depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        //depthAttachmentInfo.clearValue.depthStencil = { 1.0f, 0 };

        //VkRenderingInfo renderingInfo{};
        //renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        //renderingInfo.renderArea = { {0, 0}, {m_Window.GetWidth(), m_Window.GetHeight()} };
        //renderingInfo.layerCount = 1;
        //renderingInfo.pDepthAttachment = &depthAttachmentInfo;

        //vkCmdBeginRendering(commandBuffer, &renderingInfo);

        //// Bind pipeline with only vertex shader, depth test/write enabled
        //vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_DepthOnlyPipeline.GetGraphicsPipeline());
        ////DrawOpaqueGeometry();

        //vkCmdEndRendering(commandBuffer);
    }

    m_BaseRenderPass.RecordCommandBuffer(commandBuffer, imageIndex, &m_Scene);

	//m_SwapChain.GetDepthImage()->TransitionImageLayout(commandBuffer, m_SwapChain.GetDepthImage()->GetFormat(), m_SwapChain.GetDepthImage()->GetImageLayout(),VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE,VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT);
	//m_SwapChain.GetImages()[imageIndex].TransitionImageLayout(commandBuffer, m_SwapChain.GetImageFormat(), m_SwapChain.GetImages()[imageIndex].GetImageLayout(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_2_NONE, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_PIPELINE_STAGE_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT);

 //   VkRenderingAttachmentInfo colorAttachment = {};
	//colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	//colorAttachment.imageView = m_SwapChain.GetImages()[imageIndex].GetImageView();
	//colorAttachment.imageLayout = m_SwapChain.GetImages()[imageIndex].GetImageLayout();
	//colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	//colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
 //   colorAttachment.clearValue.color = { {0.0f, 0.0f, 0.0f, 1.0f} };

	//VkRenderingAttachmentInfo depthAttachment = {};
	//depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	//depthAttachment.imageView = m_SwapChain.GetDepthImage()->GetImageView();
	//depthAttachment.imageLayout = m_SwapChain.GetDepthImage()->GetImageLayout();
	//depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
 //   depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
 //   depthAttachment.clearValue.depthStencil = { 1.0f, 0 };

	//VkRenderingInfo renderInfo{};
	//renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
	//renderInfo.renderArea = VkRect2D{ { 0, 0 }, m_SwapChain.GetSwapChainExtent() };
	//renderInfo.layerCount = 1;
	//renderInfo.colorAttachmentCount = 1;
	//renderInfo.pColorAttachments = &colorAttachment;
	//renderInfo.pDepthAttachment = &depthAttachment;
	//renderInfo.pStencilAttachment = nullptr;

 //   //VkCommandBuffer cmndBuffer =  m_CommandPool.BeginSingleTimeCommands();
 //   vkCmdBeginRendering(commandBuffer, &renderInfo);
 //   {
 //       for (auto model : m_Scene.GetModels())
 //       {
	//        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetGraphicsPipeline());
	//        VkViewport viewport{};
	//        viewport.x = 0.0f;
	//        viewport.y = 0.0f;
	//        viewport.width = static_cast<float>(m_SwapChain.GetSwapChainExtent().width);
	//        viewport.height = static_cast<float>(m_SwapChain.GetSwapChainExtent().height);
	//        viewport.minDepth = 0.0f;
	//        viewport.maxDepth = 1.0f;
	//        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	//        VkRect2D scissor{};
	//        scissor.offset = { 0, 0 };
	//        scissor.extent = m_SwapChain.GetSwapChainExtent();
	//        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

 //           uint32_t modelIdx{ model->GetTexture() };

	//        VkBuffer vertexBuffers[] = { model->GetVertexBuffer()->GetBuffer() };
	//        VkDeviceSize offsets[] = { 0 };
 //           vkCmdPushConstants(commandBuffer, m_Pipeline.GetPipelineLayout(), VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(uint32_t), &modelIdx);
	//        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	//        vkCmdBindIndexBuffer(commandBuffer, model->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);

 //           std::vector<VkDescriptorSet> dSets = { m_UBODescriptorSets[m_CurrentFrame], m_TextureDescriptorSet };

	//        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline.GetPipelineLayout(), 0, 2, dSets.data(), 0, nullptr);
	//        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(model->GetIndices().size()), 1, 0, 0, 0);
 //       }
 //   }
 //   vkCmdEndRendering(commandBuffer);
	////m_CommandPool.EndSingleTimeCommands(cmndBuffer);

 //   //VkCommandBuffer commandBuffer = m_CommandPool.BeginSingleTimeCommands();
 //   m_SwapChain.GetImages()[imageIndex].TransitionImageLayout(commandBuffer, m_SwapChain.GetImageFormat(), m_SwapChain.GetImages()[imageIndex].GetImageLayout(), VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_2_NONE, VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_2_NONE);
 //   //m_CommandPool.EndSingleTimeCommands(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }
}
