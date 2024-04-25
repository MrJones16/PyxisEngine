#pragma once

#include <glm/glm.hpp>
#include "Pyxis/Renderer/Texture.h"
#include "Pyxis/Core/Timestep.h"

namespace Pyxis
{
	class Entity
	{
	public:
		inline static uint32_t entityCounter;
		
		Entity(const std::string& name = "Entity");

		virtual void OnUpdate(Timestep ts);
		uint32_t GetID() { return m_ID; }

		void AddChild(const Ref<Entity>& child);
		void RemoveChild(const Ref<Entity>& child);
		
		virtual void InspectorRender();

		std::string m_Name = "Entity";
		glm::mat4 m_Transform = glm::mat4(1);
		Entity* m_Parent = nullptr;
		std::vector<Ref<Entity>> m_Children;
	private:
		const uint32_t m_ID = ++entityCounter;
	};

	/// <summary>
	/// An object with a sprite
	/// </summary>
	class EntityWithSprite : public Entity
	{
	public:
		EntityWithSprite(const std::string& name = "EntityWithSprite");
		EntityWithSprite(Ref<Texture2D> texture, const std::string& name = "EntityWithSprite");
		virtual  ~EntityWithSprite() = default;

		virtual void OnUpdate(Timestep ts) override;
		
		Ref<Texture2D> m_Texture;
	};

}