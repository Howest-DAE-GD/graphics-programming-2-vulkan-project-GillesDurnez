#pragma once
#include "Buffer.h"
#include "Device/Device.h"
#include "Resources/Image.h"
#include "Resources/Texture.h"

namespace gp2
{
	class Descriptor
	{
	public:
		Descriptor(Device* pDevice);
		~Descriptor();

	private:
		void CreateDescriptorPool();
		void CreateDescriptorSets(const Buffer& buffer, const Texture& texture);
		void CreateDescriptorSetLayout();


		Device* m_Device{};

		VkDescriptorPool m_DescriptorPool; 
		std::vector <VkDescriptorSetLayout> m_DescriptorSetLayouts;
		std::vector<VkDescriptorSet> m_DescriptorSets;

	};
}
