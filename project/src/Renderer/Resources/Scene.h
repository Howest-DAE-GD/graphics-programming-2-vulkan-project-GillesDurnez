#pragma once
#include <map>
#include <vector>

#include "Model.h"
#include "Texture.h"
#include "assimp/scene.h"

namespace gp2
{
	class Scene
	{
	public:
		Scene() = default;

		~Scene();
		Scene(const Scene&) = delete;
		Scene(Scene&&) = delete;
		Scene& operator=(const Scene&) = delete;
		Scene& operator=(Scene&&) = delete;

		void LoadScene(Device* pDevice, CommandPool* pCommandPool, const std::string& path);

		uint32_t AddModel(Model* model)
		{
			m_Models.push_back(model);
			return static_cast<uint32_t>(m_Models.size() - 1);
		}

		uint32_t AddTexture(Texture* texture, const std::string& path = "");

		Model* GetModel(uint32_t index) const
		{
			if (index < m_Models.size())
			{
				return m_Models[index];
			}
			return nullptr;
		}

		Texture* GetTexture(uint32_t index) const
		{
			if (index < m_Textures.size())
			{
				return m_Textures[index];
			}
			return nullptr;
		}

		int GetTextureIndex(const std::string& path)
		{
			if (m_LoadedTextures.contains(path))
				return m_LoadedTextures[path];
			return -1;
		}

		const std::vector<Model*>& GetModels() const { return m_Models; }
		const std::vector<Texture*>& GetTextures() const { return m_Textures; }

	private:
		void ProcessNode(Device* pDevice, CommandPool* pCommandPool, const aiNode* node, const aiScene* scene, const std::string& path);


	private:
		std::vector<Model*> m_Models{};

		std::vector<Texture*> m_Textures{};

		std::map<std::string, uint32_t> m_LoadedTextures;

		inline static Assimp::Importer m_Importer{};
	};
}
