#pragma once
#include <vulkan/vulkan.h>

#include "Image.h"
#include "../Device.h"
#include "../Buffer.h"
#include "../SwapChain.h"


namespace gp2
{
	class Texture
	{
	public:
		Texture() = default;

		Texture(Device* pDevice, CommandPool* pCommandPool, std::string path, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB);
		~Texture();

		Texture(const Texture&) = delete;
		Texture(Texture&&) noexcept;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) noexcept;

		Image* GetTextureImage() const { return m_TextureImage; }


	private:
		Device* m_pDevice;
		CommandPool* m_pCommandPool;

		Image* m_TextureImage{ nullptr };
	};
}