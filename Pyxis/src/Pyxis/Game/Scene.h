#pragma once

#include "Pyxis/Core/Core.h"
#include "Pyxis/Renderer/Camera.h"
#include "Pyxis/Game/Entity.h"

namespace Pyxis
{
	class Scene
	{
	public:
		Scene();
		
		void AddEntity(const Ref<Entity>& entity);
		void RemoveEntity(const Ref<Entity>& entity);

		void Update(Timestep ts);

		std::unordered_map<uint32_t, Ref<Entity>> GetEntities() { return m_Entities; }
		//std::vector<Ref<Entity>> GetEntities() { return m_Entities; }

		OrthographicCamera* m_ActiveCamera;
		std::unordered_map<uint32_t, Ref<Entity>> m_Entities;
	private:
		//std::vector<Ref<Entity>> m_Entities;
	};
}