#pragma once
#include <vulkan/vulkan_core.h>

#include "../Device.h"

namespace gp2
{
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
			VkMemoryPropertyFlags properties;
		};

		Image(const Device* pDevice, 
			uint32_t width, 
			uint32_t height, 
			VkFormat format, 
			VkImageTiling tiling, 
			VkImageUsageFlags usage, 
			VkMemoryPropertyFlags properties);
		Image(const Device* pDevice, const ImageCreateInfo& imageCreate);
		~Image();

		Image(const Image&) = delete;
		Image(Image&&) = default;
		Image& operator=(const Image&) = delete;
		Image& operator=(Image&&) = default;

		VkDeviceMemory* GetImageMemory() { return &m_ImageMemory; }
		VkImage* GetImage() { return &m_Image; }
		VkImageView* GetImageView() { return &m_ImageView; }

		void TransitionImageLayout(VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height) const;
		void CreateImageView(VkFormat format, VkImageAspectFlags aspectFlags);

	private:

		void CreateImage(const ImageCreateInfo& imageCreateInfo);

	protected:
		const Device* m_pDevice;

		VkImage m_Image;
		VkDeviceMemory m_ImageMemory;
		VkImageView m_ImageView;
	};
}
