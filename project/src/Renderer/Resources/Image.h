#pragma once
#include <vulkan/vulkan_core.h>

#include "../Device.h"

namespace gp2
{
	class CommandPool;

	class Image
	{
	public:
		struct ImageCreateInfo
		{
			uint32_t width;
			uint32_t height;
			VkFormat format;
			VkImageTiling tiling;
			VkImageUsageFlags usage;
			VkImageAspectFlags aspectFlags;
			VkMemoryPropertyFlags properties;
		};

		Image(const Device* pDevice, const CommandPool* pCommandPool,
			uint32_t width, 
			uint32_t height, 
			VkFormat format, 
			VkImageTiling tiling, 
			VkImageUsageFlags usage,
			VkImageAspectFlags aspectFlags,
			VkMemoryPropertyFlags properties);
		Image(const Device* pDevice, const CommandPool* pCommandPool, const ImageCreateInfo& imageCreate);
		Image(const Device* pDevice, const CommandPool* pCommandPool, const VkImageCreateInfo& vkImageCreateInfo, const VkMemoryPropertyFlags& properties, const VkFormat& format, const VkImageAspectFlags& aspectFlags);
		~Image();

		Image(const Image&) = delete;
		Image(Image&&) = default;
		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = default;

		VkImage GetImage() const { return m_Image; }
		VkImageView GetImageView() const { return m_ImageView; }
		VmaAllocation GetImageAllocation() const { return m_ImageAllocation; }

		void TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const;
		void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height) const;
		void CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags);

	private:

		void CreateImage(const ImageCreateInfo& imageCreateInfo);
		void CreateImage(const VkImageCreateInfo& imageCreateInfo, const VkMemoryPropertyFlags& properties);

	protected:
		const Device* m_pDevice;
		const CommandPool* m_pCommandPool;

		VkImage m_Image;
		VmaAllocation m_ImageAllocation;
		VkImageView m_ImageView;
	};
}
