#include "Shader.h"

#include <fstream>
#include <stdexcept>


gp2::Shader::Shader(Device* pDevice, const std::string& filePath)
	: m_pDevice(pDevice)
{
	m_ShaderModule = CreateShaderModule(pDevice->GetLogicalDevice(), ReadFile(filePath));
}

gp2::Shader::Shader(Shader&& other) noexcept
{
    m_pDevice = other.m_pDevice;

    m_ShaderModule = other.m_ShaderModule;
    other.m_ShaderModule = VK_NULL_HANDLE;
}

gp2::Shader& gp2::Shader::operator=(Shader&& other) noexcept
{
    m_pDevice = other.m_pDevice;

    m_ShaderModule = other.m_ShaderModule;
    other.m_ShaderModule = VK_NULL_HANDLE;

    return *this;
}

gp2::Shader::~Shader()
{
    if (m_ShaderModule != VK_NULL_HANDLE && m_pDevice != nullptr)
		vkDestroyShaderModule(m_pDevice->GetLogicalDevice(), m_ShaderModule, nullptr);
}

VkShaderModule gp2::Shader::CreateShaderModule(const VkDevice& logicalDevice, const std::vector<char>& code)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

std::vector<char> gp2::Shader::ReadFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file!");
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buffer(fileSize);

    file.seekg(0);
    file.read(buffer.data(), fileSize);

    file.close();

    return buffer;
}
