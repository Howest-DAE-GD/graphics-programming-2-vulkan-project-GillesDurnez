#include "Texture.h"

#include "Image.h"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


gp2::Texture::Texture(Device* pDevice, CommandPool* pCommandPool, std::string path)
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = imageSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    Buffer stagingBuffer{ m_pDevice, m_pCommandPool, bufferInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT , true};

    vmaCopyMemoryToAllocation(m_pDevice->GetAllocator(), pixels, stagingBuffer.GetBufferAllocation(), 0, imageSize);

	m_TextureImage = new Image{ m_pDevice, m_pCommandPool, static_cast<unsigned int>(texWidth), static_cast<unsigned int>(texHeight), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
	vmaSetAllocationName(m_pDevice->GetAllocator(), m_TextureImage->GetImageAllocation(), "Texture image Buffer");

	m_TextureImage->TransitionImageLayout(
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	m_TextureImage->CopyBufferToImage(stagingBuffer.GetBuffer(), texWidth, texHeight);
	m_TextureImage->TransitionImageLayout(
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);
	//m_TextureImage->TransitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	stbi_image_free(pixels);
}

gp2::Texture::~Texture()
{
	if (m_TextureImage != nullptr)
	{
		delete m_TextureImage;
		m_TextureImage = nullptr;
	}
}
