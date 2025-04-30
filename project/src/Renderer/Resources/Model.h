#pragma once
#include <array>
#include <cstddef>
#include <string>

#include <vulkan/vulkan_core.h>

#define GLM_FORCE_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include "Texture.h"
#include "../Buffer.h"
//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>
//#include <unordered_map>


namespace gp2
{
	struct Vertex
	{
		glm::vec3 pos;
	    glm::vec3 color;
	    glm::vec2 texCoord;

	    static VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
	        bindingDescription.binding = 0;
	        bindingDescription.stride = sizeof(Vertex);
	        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	        return bindingDescription;
	    }

	    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
		{
	        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

	        attributeDescriptions[0].binding = 0;
	        attributeDescriptions[0].location = 0;
	        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	        attributeDescriptions[0].offset = offsetof(Vertex, pos);


	        attributeDescriptions[1].binding = 0;
	        attributeDescriptions[1].location = 1;
	        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	        attributeDescriptions[1].offset = offsetof(Vertex, color);

	        attributeDescriptions[2].binding = 0;
	        attributeDescriptions[2].location = 2;
	        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

	        return attributeDescriptions;
	    }

	    bool operator==(const Vertex& other) const
		{
	        return pos == other.pos && color == other.color && texCoord == other.texCoord;
	    }
	};

	class Model
	{
	public:
		Model() = default;

		Model(Device* pDevice, CommandPool* pCommandPool, const std::string& path);
		~Model();

		Model(const Model&) = delete;
		Model(Model&&) noexcept;
		Model& operator=(const Model&) = delete;
		Model& operator=(Model&&) noexcept;

		const std::vector<Vertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }

		const Buffer* GetVertexBuffer() const { return m_VertexBuffer; }
		const Buffer* GetIndexBuffer() const { return m_IndexBuffer; }

		void AddTexture(uint32_t index) { m_TextureIndexes.push_back(index); }

	private:
        Device* m_pDevice;
		CommandPool* m_pCommandPool;

		std::vector<uint32_t> m_TextureIndexes;

        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;

        Buffer* m_VertexBuffer;
        Buffer* m_IndexBuffer;

        void LoadModel(const std::string& path);
        void CreateVertexBuffer();
        void CreateIndexBuffer();
	};
}

