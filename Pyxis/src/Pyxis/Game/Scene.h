#pragma once

#include "Pyxis/Core/Core.h"
#include "Pyxis/Renderer/Camera.h"
#include "Pyxis/Game/Node.h"

namespace Pyxis
{
	class Scene
	{
	public:
		Scene();
		~Scene() = default;
		
		void AddNode(const Ref<Node>& Node);
		void RemoveNode(const Ref<Node>& Node);

		void Update(Timestep ts);
		void Render();

		std::unordered_map<uint32_t, Ref<Node>> GetNodes() { return m_Nodes; }
		//std::vector<Ref<Node>> GetNodes() { return m_Nodes; }

		Ref<OrthographicCamera> m_ActiveCamera;
		std::unordered_map<uint32_t, Ref<Node>> m_Nodes;

		uint32_t m_HoveredNodeID;
	private:
		//std::vector<Ref<Node>> m_Nodes;
	};
}