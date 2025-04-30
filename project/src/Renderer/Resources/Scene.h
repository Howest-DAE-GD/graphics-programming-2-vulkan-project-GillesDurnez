#pragma once
#include <vector>

#include "Model.h"
#include "Texture.h"

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

		uint32_t AddModel(Model* model)
		{
			m_Models.push_back(model);
			return static_cast<uint32_t>(m_Models.size() - 1);
		}

		uint32_t AddTexture(Texture* texture)
		{
			m_Textures.push_back(texture);
			return static_cast<uint32_t>(m_Textures.size() - 1);
		}

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

		const std::vector<Model*>& GetModels() const { return m_Models; }
		const std::vector<Texture*>& GetTextures() const { return m_Textures; }

	private:
		std::vector<Model*> m_Models{};
		std::vector<Texture*> m_Textures{};
	};
}