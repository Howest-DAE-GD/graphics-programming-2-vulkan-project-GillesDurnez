#include "Model.h"
//
//gp2::Model::Model(Device* pDevice, std::string path)
//	: m_pDevice(pDevice)
//{
//	LoadModel(path);
//	CreateVertexBuffer();
//	CreateIndexBuffer();
//}
//
//gp2::Model::~Model()
//{
//
//}
//
//void gp2::Model::LoadModel(std::string path)
//{
//    tinyobj::attrib_t attrib;
//    std::vector<tinyobj::shape_t> shapes;
//    std::vector<tinyobj::material_t> materials;
//    std::string warn, err;
//
//    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
//    {
//        throw std::runtime_error(warn + err);
//    }
//
//    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
//
//
//    for (const auto& shape : shapes)
//    {
//        for (const auto& index : shape.mesh.indices)
//        {
//            Vertex vertex{};
//
//            vertex.pos = {
//                attrib.vertices[3 * index.vertex_index + 0],
//                attrib.vertices[3 * index.vertex_index + 1],
//                attrib.vertices[3 * index.vertex_index + 2]
//            };
//
//            vertex.texCoord = {
//                attrib.texcoords[2 * index.texcoord_index + 0],
//                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
//            };
//
//            vertex.color = { 1.0f, 1.0f, 1.0f };
//
//            if (uniqueVertices.count(vertex) == 0)
//            {
//                uniqueVertices[vertex] = static_cast<uint32_t>(m_Vertices.size());
//                m_Vertices.push_back(vertex);
//            }
//
//            m_Indices.push_back(uniqueVertices[vertex]);
//        }
//    }
//}
//
//void gp2::Model::CreateVertexBuffer()
//{
//    VkDeviceSize bufferSize = sizeof(m_Vertices[0]) * m_Vertices.size();
//
//    Buffer stagingBuffer{ m_pDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
//
//    void* data;
//    vkMapMemory(m_pDevice->GetLogicalDevice(), stagingBuffer.GetBufferMemory(), 0, bufferSize, 0, &data);
//    memcpy(data, m_Vertices.data(), (size_t)bufferSize);
//    vkUnmapMemory(m_pDevice->GetLogicalDevice(), stagingBuffer.GetBufferMemory());
//
//    m_VertexBuffer = new Buffer{ m_pDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
//    m_VertexBuffer->CopyBuffer(stagingBuffer.GetBuffer(), bufferSize);
//}
//
//void gp2::Model::CreateIndexBuffer()
//{
//    VkDeviceSize bufferSize = sizeof(m_Indices[0]) * m_Indices.size();
//
//    Buffer stagingBuffer{ m_pDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
//
//    void* data;
//    vkMapMemory(m_pDevice->GetLogicalDevice(), stagingBuffer.GetBufferMemory(), 0, bufferSize, 0, &data);
//    memcpy(data, m_Indices.data(), (size_t)bufferSize);
//    vkUnmapMemory(m_pDevice->GetLogicalDevice(), stagingBuffer.GetBufferMemory());
//
//    m_IndexBuffer = new Buffer{ m_pDevice, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
//    m_IndexBuffer->CopyBuffer(stagingBuffer.GetBuffer(), bufferSize);
//}
//
