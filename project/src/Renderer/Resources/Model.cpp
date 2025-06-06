#include "Model.h"
//
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <unordered_map>

#include <glm/glm.hpp>

//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <stdexcept>

namespace std
{
    template<> struct hash<gp2::Vertex>
    {
        size_t operator()(gp2::Vertex const& vertex) const
        {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

gp2::Model::Model(Device* pDevice, CommandPool* pCommandPool, aiMesh* mesh, const aiMatrix4x4& transform )
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool)
{
	m_Vertices.reserve(mesh->mNumVertices);
	m_Indices.reserve(mesh->mNumFaces * 3);

	for (uint32_t i = 0; i < mesh->mNumVertices; i++)
	{

		Vertex vertex{};
		vertex.pos = {
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		};

        vertex.pos = ToGlmMat(transform) * glm::vec4(vertex.pos, 1.f);

        // NORMALS
        if (mesh->HasNormals()) {
            vertex.normal = {
                mesh->mNormals[i].x,
                mesh->mNormals[i].y,
                mesh->mNormals[i].z
            };
		}
		else
		{
			vertex.normal = { 1.0f, 1.0f, 1.0f };
		}

        if (mesh->HasTangentsAndBitangents())
        {
            vertex.tangent = {
                mesh->mTangents[i].x,
                mesh->mTangents[i].y,
                mesh->mTangents[i].z
            };
        }
        else
        {
			vertex.tangent = { 1.0f, 1.0f, 1.0f }; // Default value if no tangents are present
        }

		// UV COORDINATES
		if (mesh->HasTextureCoords(0))
		{
			vertex.texCoord = {
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			};
		}
		else
		{
			vertex.texCoord = { 0.0f, 0.0f };
		}

		m_Vertices.push_back(vertex);
	}

	for (uint32_t faceIdx = 0; faceIdx < mesh->mNumFaces; faceIdx++)
	{
		for (uint32_t indeceIdx = 0; indeceIdx < mesh->mFaces[faceIdx].mNumIndices; indeceIdx++)
		{
			m_Indices.push_back(mesh->mFaces[faceIdx].mIndices[indeceIdx]);
		}
	}

    CreateVertexBuffer();
    CreateIndexBuffer();
}

gp2::Model::Model(Device* pDevice, CommandPool* pCommandPool, const std::string& path)
	: m_pDevice(pDevice), m_pCommandPool(pCommandPool)
{
	LoadModel(path);
	CreateVertexBuffer();
	CreateIndexBuffer();
}

gp2::Model::~Model()
{
	delete m_VertexBuffer;
	delete m_IndexBuffer;

	m_VertexBuffer = nullptr;
	m_IndexBuffer = nullptr;
}

gp2::Model::Model(Model&& other) noexcept
{
	m_Vertices = std::move(other.m_Vertices);
	m_Indices = std::move(other.m_Indices);
	m_VertexBuffer = std::move(other.m_VertexBuffer);
	m_IndexBuffer = std::move(other.m_IndexBuffer);
	m_pDevice = std::move(other.m_pDevice);
	m_pCommandPool = std::move(other.m_pCommandPool);
	m_TextureIndexes = std::move(other.m_TextureIndexes);

	other.m_VertexBuffer = nullptr;
	other.m_IndexBuffer = nullptr;
}

gp2::Model& gp2::Model::operator=(Model&& other) noexcept
{
    m_Vertices = std::move(other.m_Vertices);
    m_Indices = std::move(other.m_Indices);
    m_VertexBuffer = std::move(other.m_VertexBuffer);
    m_IndexBuffer = std::move(other.m_IndexBuffer);
    m_pDevice = std::move(other.m_pDevice);
    m_pCommandPool = std::move(other.m_pCommandPool);
    m_TextureIndexes = std::move(other.m_TextureIndexes);

    other.m_VertexBuffer = nullptr;
    other.m_IndexBuffer = nullptr;

    return *this;
}

void gp2::Model::LoadModel(const std::string& path)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
    {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<gp2::Vertex, uint32_t> uniqueVertices{};


    for (const auto& shape : shapes)
    {
        for (const auto& index : shape.mesh.indices)
        {
            gp2::Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.normal = { 1.0f, 1.0f, 1.0f };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
                m_Vertices.push_back(vertex);
            }

            m_Indices.push_back(uniqueVertices[vertex]);
        }
    }
}

void gp2::Model::CreateVertexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = bufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;


    //VkBuffer stagingBuffer;
    Buffer stagingBuffer{ m_pDevice, m_pCommandPool, bufferInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true };

    vmaCopyMemoryToAllocation(m_pDevice->GetAllocator(), m_Vertices.data(), stagingBuffer.GetBufferAllocation(), 0, bufferSize);

	VkBufferCreateInfo vertexBufferInfo{};
	vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferInfo.size = bufferSize;
	vertexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    m_VertexBuffer = new Buffer{ m_pDevice, m_pCommandPool, vertexBufferInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
    m_VertexBuffer->CopyBuffer(stagingBuffer.GetBuffer(), bufferSize);
}

void gp2::Model::CreateIndexBuffer()
{
    VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();

    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    Buffer stagingBuffer{ m_pDevice, m_pCommandPool, bufferInfo, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true };

    vmaCopyMemoryToAllocation(m_pDevice->GetAllocator(), m_Indices.data(), stagingBuffer.GetBufferAllocation(), 0, bufferSize);

    VkBufferCreateInfo indexBufferInfo{};
    indexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    indexBufferInfo.size = bufferSize;
    indexBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

    m_IndexBuffer = new Buffer{ m_pDevice, m_pCommandPool, indexBufferInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
    m_IndexBuffer->CopyBuffer(stagingBuffer.GetBuffer(), bufferSize);
}

glm::mat4 gp2::Model::ToGlmMat(const aiMatrix4x4& mat)
{
    // Thank u Kobe
    return glm::mat4(
        mat.a1, mat.b1, mat.c1, mat.d1,
        mat.a2, mat.b2, mat.c2, mat.d2,
        mat.a3, mat.b3, mat.c3, mat.d3,
        mat.a4, mat.b4, mat.c4, mat.d4
    );
}
