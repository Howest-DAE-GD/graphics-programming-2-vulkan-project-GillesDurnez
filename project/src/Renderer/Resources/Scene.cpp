#include "Scene.h"

#include <filesystem>

#include "assimp/postprocess.h"
#include "assimp/scene.h"

gp2::Scene::~Scene()
{
	for (auto model : m_Models)
	{
		delete model;
	}

	for (auto texture : m_Textures)
	{
		delete texture;
	}

	m_Models.clear();
	m_Textures.clear();
}

void gp2::Scene::LoadScene(Device* pDevice, CommandPool* pCommandPool, const std::string& path)
{
	const aiScene* scene = m_Importer.ReadFile(
		path,
		aiProcess_Triangulate |
		aiProcess_GenSmoothNormals |
		aiProcess_FlipUVs |
		aiProcess_JoinIdenticalVertices
	);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cerr << "ERROR::ASSIMP::" << m_Importer.GetErrorString() << "\n";
	}

	std::filesystem::path fsDataPath{ path };
	std::string dataPath{ fsDataPath.parent_path().string() + "/" };

	ProcessNode(pDevice, pCommandPool ,scene->mRootNode, scene, dataPath);
}

uint32_t gp2::Scene::AddTexture(Texture* texture, const std::string& path)
{
	m_Textures.emplace_back(texture);
	m_LoadedTextures.insert(std::pair<std::string, uint32_t>(path, m_Textures.size() - 1));

	return static_cast<uint32_t>(m_Textures.size() - 1);
}

void gp2::Scene::ProcessNode(Device* pDevice, CommandPool* pCommandPool, const aiNode* node, const aiScene* scene, const std::string& path)
{
	// Process each mesh at this node
	for (unsigned int i = 0; i < node->mNumMeshes; i++) 
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		Model* newModel = new Model{ pDevice, pCommandPool, mesh };
		m_Models.push_back(newModel);

		aiString texturePath;
		scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath);
		std::string relativeTexturePath = path + texturePath.C_Str();

		aiString normalMapPath;
		scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_NORMALS, 0, &normalMapPath);
		std::string relativeNormalMapPath = path + normalMapPath.C_Str();

		aiString metalnesPath;
		scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_METALNESS, 0, &metalnesPath);
		std::string relativeMetalnesPath = path + metalnesPath.C_Str();

		aiString roughnessPath;
		scene->mMaterials[mesh->mMaterialIndex]->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &roughnessPath);
		std::string relativeRoughnessPath = path + roughnessPath.C_Str();

		// +----- Textures -----+
		if (m_LoadedTextures.contains(relativeTexturePath))
		{
			newModel->AddTexture(m_LoadedTextures[relativeTexturePath]);
		}
		else
		{
			m_Textures.emplace_back(new Texture{ pDevice, pCommandPool, relativeTexturePath });
			m_LoadedTextures.insert(std::pair<std::string, uint32_t>(relativeTexturePath, m_Textures.size() - 1));
			newModel->AddTexture(m_LoadedTextures[relativeTexturePath]);

		}

		// +----- Normal Maps -----+
		if (normalMapPath.length != 0)
		{
			if (m_LoadedTextures.contains(relativeNormalMapPath))
			{
				newModel->AddNormalMap(m_LoadedTextures[relativeNormalMapPath]);
			}
			else
			{
				m_Textures.emplace_back(new Texture{ pDevice, pCommandPool, relativeNormalMapPath });
				m_LoadedTextures.insert(std::pair<std::string, uint32_t>(relativeNormalMapPath, m_Textures.size() - 1));
				newModel->AddNormalMap(m_LoadedTextures[relativeNormalMapPath]);

			}
		}

		// +----- Metalness maps -----+
		if (metalnesPath.length != 0)
		{
			if (m_LoadedTextures.contains(relativeMetalnesPath))
			{
				newModel->AddMetalMap(m_LoadedTextures[relativeMetalnesPath]);
			}
			else
			{
				m_Textures.emplace_back(new Texture{ pDevice, pCommandPool, relativeMetalnesPath });
				m_LoadedTextures.insert(std::pair<std::string, uint32_t>(relativeMetalnesPath, m_Textures.size() - 1));
				newModel->AddMetalMap(m_LoadedTextures[relativeMetalnesPath]);

			}
		}

		// +----- Roughness Maps -----+
		if (roughnessPath.length != 0)
		{
			if (m_LoadedTextures.contains(relativeRoughnessPath))
			{
				newModel->AddRoughnessMap(m_LoadedTextures[relativeRoughnessPath]);
			}
			else
			{
				m_Textures.emplace_back(new Texture{ pDevice, pCommandPool, relativeRoughnessPath });
				m_LoadedTextures.insert(std::pair<std::string, uint32_t>(relativeRoughnessPath, m_Textures.size() - 1));
				newModel->AddRoughnessMap(m_LoadedTextures[relativeRoughnessPath]);

			}
		}
	}

	// Then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++) 
	{
		ProcessNode(pDevice, pCommandPool, scene->mRootNode, scene, path);
	}
}
