#include "Image.h"

#include <stdexcept>

//#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "../CommandPool.h"

gp2::Image::Image(const Device* pDevice, const CommandPool* pCommandPool, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
                  VkImageUsageFlags usage, VkImageAspectFlags aspectFlags, VkMemoryPropertyFlags properties)
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool), m_Format(format)
{
	ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.width = width;
	imageCreateInfo.height = height;
	imageCreateInfo.format = format;
	imageCreateInfo.tiling = tiling;
	imageCreateInfo.usage = usage;
	imageCreateInfo.aspectFlags = aspectFlags;
	imageCreateInfo.properties = properties;

	CreateImage(imageCreateInfo);
    CreateImageView(imageCreateInfo.format, imageCreateInfo.aspectFlags);
}

gp2::Image::Image(const Device* pDevice, const CommandPool* pCommandPool, const ImageCreateInfo& imageCreate)
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool), m_Format(imageCreate.format), m_ImageAspectFlags(imageCreate.aspectFlags)
{
	CreateImage(imageCreate);
	CreateImageView(imageCreate.format, imageCreate.aspectFlags);
}

gp2::Image::Image(const Device* pDevice, const CommandPool* pCommandPool, const VkImageCreateInfo& vkImageCreateInfo,
	const VkMemoryPropertyFlags& properties, const VkFormat& format, const VkImageAspectFlags& aspectFlags)
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool), m_Format(format), m_ImageAspectFlags(aspectFlags)
{
    CreateImage(vkImageCreateInfo, properties);
    CreateImageView(format, aspectFlags);
}

gp2::Image::Image(const Device* pDevice, const CommandPool* pCommandPool, VkImage image, const VkFormat& format, const VkImageAspectFlags& aspectFlags)
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool), m_Format(format), m_ImageAspectFlags(aspectFlags)
{
	m_Image = image;
	m_ImageAllocation = VK_NULL_HANDLE;
    CreateImageView(format, aspectFlags);
}

gp2::Image::~Image()
{
    CleanupImageView();
	if (m_Image != VK_NULL_HANDLE && m_ImageAllocation != VK_NULL_HANDLE)
		vmaDestroyImage(m_pDevice->GetAllocator(), m_Image, m_ImageAllocation);
}

gp2::Image::Image(Image&& other) noexcept
{
	m_Image = other.m_Image;
	m_ImageAllocation = other.m_ImageAllocation;
	m_ImageView = other.m_ImageView;
	m_pDevice = other.m_pDevice;
	m_pCommandPool = other.m_pCommandPool;
	m_Format = other.m_Format;
	m_ImageAspectFlags = other.m_ImageAspectFlags;

	other.m_Image = VK_NULL_HANDLE;
	other.m_ImageAllocation = VK_NULL_HANDLE;
	other.m_ImageView = VK_NULL_HANDLE;
}

gp2::Image& gp2::Image::operator=(Image&& other) noexcept
{
    m_Image = other.m_Image;
    m_ImageAllocation = other.m_ImageAllocation;
    m_ImageView = other.m_ImageView;
    m_pDevice = other.m_pDevice;
    m_pCommandPool = other.m_pCommandPool;
	m_Format = other.m_Format;
	m_ImageAspectFlags = other.m_ImageAspectFlags;

    other.m_Image = VK_NULL_HANDLE;
    other.m_ImageAllocation = VK_NULL_HANDLE;
    other.m_ImageView = VK_NULL_HANDLE;


	return *this;
}


void gp2::Image::CleanupImageView()
{
    if (m_ImageView == VK_NULL_HANDLE) return;

	vkDestroyImageView(m_pDevice->GetLogicalDevice(), m_ImageView, nullptr);
	m_ImageView = VK_NULL_HANDLE;
}

//void gp2::Image::RecreateImageView()
//{
//    CleanupImageView();
//	CreateImageView(m_Format, m_ImageAspectFlags);
//}

void gp2::Image::TransitionImageLayout(VkCommandBuffer& commandBuffer, VkFormat format, VkImageLayout newLayout)
{
	TransitionImageLayout(commandBuffer, format, m_ImageLayout, newLayout);
}

void gp2::Image::TransitionImageLayout(VkCommandBuffer& commandBuffer, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkAccessFlags srcAccessMask;
	VkAccessFlags dstAccessMask;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        srcAccessMask = 0;
    	dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        srcAccessMask = 0;
        dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("unsupported layout transition!");
    }

    TransitionImageLayout(commandBuffer, format, oldLayout, newLayout, srcAccessMask, dstAccessMask, sourceStage, destinationStage);
}

void gp2::Image::TransitionImageLayout(VkCommandBuffer& commandBuffer, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
	VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkPipelineStageFlags sourceStage, VkPipelineStageFlags destinationStage, VkImageAspectFlags imageAspectFlags)
{

	m_ImageLayout = newLayout;

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_Image;
    barrier.subresourceRange.aspectMask = imageAspectFlags;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (m_pDevice->HasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }



    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );



}

void gp2::Image::CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height) const
{
    VkCommandBuffer commandBuffer = m_pCommandPool->BeginSingleTimeCommands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };

        vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        m_Image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    m_pCommandPool->EndSingleTimeCommands(commandBuffer);
}

void gp2::Image::CreateImage(const ImageCreateInfo& imageCreateInfo)
{

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = imageCreateInfo.width;
    imageInfo.extent.height = imageCreateInfo.height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = imageCreateInfo.format;
    imageInfo.tiling = imageCreateInfo.tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = imageCreateInfo.usage;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	CreateImage(imageInfo, imageCreateInfo.properties);
}

void gp2::Image::CreateImage(const VkImageCreateInfo& imageCreateInfo, const VkMemoryPropertyFlags& properties)
{

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocInfo.requiredFlags = properties;
	m_ImageLayout = imageCreateInfo.initialLayout;

    vmaCreateImage(m_pDevice->GetAllocator(), &imageCreateInfo, &allocInfo, &m_Image, &m_ImageAllocation, nullptr);
}

void gp2::Image::CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags)
{
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_Image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_pDevice->GetLogicalDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
}
