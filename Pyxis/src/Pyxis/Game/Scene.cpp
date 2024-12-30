#include "pxpch.h"

#include "Scene.h"

namespace Pyxis
{
	Scene::Scene()
	{
		m_ActiveCamera = nullptr;
		m_Nodes = std::unordered_map<uint32_t, Ref<Node>>();
		//m_Nodes = std::vector<Ref<Node>>();
	}
	void Scene::AddNode(const Ref<Node>& Node)
	{
		m_Nodes.emplace(Node->GetID(), Node);
		//m_Nodes.insert(Node->GetID(), Node);
		//m_Nodes.push_back(Node);
		//m_Prototypes.insert()
	}

	void Scene::RemoveNode(const Ref<Node>& Node)
	{
		for (auto it = m_Nodes.begin(); it != m_Nodes.end(); it++)
		{
			if (it->second == Node)
			{
				m_Nodes.erase(it);
				break;
			}
		}
	}

	void Scene::Update(Timestep ts)
	{
		for (std::pair<uint32_t, Ref<Node>> NodePair : m_Nodes)
		{
			NodePair.second->OnUpdate(ts);
		}
	}

	void Scene::Render()
	{
		for (std::pair<uint32_t, Ref<Node>> NodePair : m_Nodes)
		{
			NodePair.second->OnRender();
		}
	}
}