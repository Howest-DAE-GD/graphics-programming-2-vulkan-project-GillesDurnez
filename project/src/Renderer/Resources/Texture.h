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
		Texture(Device* pDevice, CommandPool* pCommandPool, std::string path);
		~Texture();

		Texture(const Texture&) = delete;
		Texture(Texture&&) = default;
		Texture& operator=(const Texture&) = delete;
		Texture& operator=(Texture&&) = default;

		Image* GetTextureImage() const { return m_TextureImage; }


	private:
		Device* m_pDevice;
		CommandPool* m_pCommandPool;

		Image* m_TextureImage{ nullptr };
	};
}