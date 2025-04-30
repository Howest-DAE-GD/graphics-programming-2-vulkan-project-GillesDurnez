#include "Scene.h"

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
