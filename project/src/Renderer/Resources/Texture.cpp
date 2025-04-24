#include "Texture.h"

#include "Image.h"

#include <stdexcept>
#include <vulkan/vulkan_core.h>

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


gp2::Texture::Texture(Device* pDevice, SwapChain* pSwapChain, std::string path)
	: m_pDevice(pDevice)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    Buffer stagingBuffer{ m_pDevice ,imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };

    void* data;
    vkMapMemory(m_pDevice->GetLogicalDevice(), stagingBuffer.GetBufferMemory(), 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_pDevice->GetLogicalDevice(), stagingBuffer.GetBufferMemory());

    stbi_image_free(pixels);

	m_TextureImage = new Image{ m_pDevice, static_cast<unsigned int>(texWidth), static_cast<unsigned int>(texHeight), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

    pSwapChain->TransitionImageLayout(*m_TextureImage->GetImage(), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_TextureImage->CopyBufferToImage(stagingBuffer.GetBuffer(), static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));

    m_TextureImage->TransitionImageLayout( VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    m_TextureImage->CreateImageView(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
	CreateTextureSampler();
}

gp2::Texture::~Texture()
{
	if (m_TextureSampler != VK_NULL_HANDLE)
	{
		vkDestroySampler(m_pDevice->GetLogicalDevice(), m_TextureSampler, nullptr);
	}

	if (m_TextureImage != nullptr)
	{
		delete m_TextureImage;
		m_TextureImage = nullptr;
	}
}

void gp2::Texture::CreateTextureSampler()
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
    vkGetPhysicalDeviceProperties(m_pDevice->GetPhysicalDevice(), &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_pDevice->GetLogicalDevice(), &samplerInfo, nullptr, &m_TextureSampler) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture sampler!");
    }
}
