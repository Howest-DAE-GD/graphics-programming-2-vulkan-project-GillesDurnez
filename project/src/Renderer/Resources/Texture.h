#pragma once
#include <vulkan/vulkan.h>

#include "Image.h"
#include "../Device/Device.h"
#include "../Buffer.h"
#include "../SwapChain.h"


namespace gp2
{
	class Texture
	{
	public:
		Texture(Device* pDevice, SwapChain* pSwapChain, std::string path);
		~Texture();

		VkSampler GetSampler() const { return m_TextureSampler; }

		VkImage GetImage() const { return *m_TextureImage->GetImage(); }
		VkImageView GetImageView() const { return *m_TextureImage->GetImageView(); }
		VkDeviceMemory GetImageMemory() const { return *m_TextureImage->GetImageMemory(); }

		void CreateTextureSampler();

	private:
		Device* m_pDevice;

		Image* m_TextureImage;
		VkSampler m_TextureSampler;
	};
}