#include "pxpch.h"

#include "Scene.h"

namespace Pyxis
{
	Scene::Scene()
	{
		m_ActiveCamera = nullptr;
		m_Entities = std::unordered_map<uint32_t, Ref<Entity>>();
		//m_Entities = std::vector<Ref<Entity>>();
	}
	void Scene::AddEntity(const Ref<Entity>& entity)
	{
		m_Entities.emplace(entity->GetID(), entity);
		//m_Entities.insert(entity->GetID(), entity);
		//m_Entities.push_back(entity);
		//m_Prototypes.insert()
	}
	void Scene::RemoveEntity(const Ref<Entity>& entity)
	{
		for (auto it = m_Entities.begin(); it != m_Entities.end(); it++)
		{
			if (it->second == entity)
			{
				m_Entities.erase(it);
				break;
			}
		}
	}
	void Scene::Update(Timestep ts)
	{
		for each (auto entity in m_Entities)
		{
			entity.second->OnUpdate(ts);
		}
	}
}