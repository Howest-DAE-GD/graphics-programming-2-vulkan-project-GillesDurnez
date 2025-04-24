#pragma once
#include <vulkan/vulkan.h>

#include <string>
#include <vector>

#include "../Device.h"


namespace gp2 
{
	class Shader
	{
	public:
		Shader(Device* pDevice, const std::string& filePath);
		~Shader();
		VkShaderModule GetShaderModule() const { return m_ShaderModule; }

		static VkShaderModule CreateShaderModule(const VkDevice& logicalDevice, const std::vector<char>& code);
		static std::vector<char> ReadFile(const std::string& filePath);
	private:
		Device* m_pDevice{};

		VkShaderModule m_ShaderModule{};
	};
}
