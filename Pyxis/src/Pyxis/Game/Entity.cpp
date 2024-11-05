#include "pxpch.h"
#include "Entity.h"
#include "Pyxis/Renderer/Renderer2D.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

namespace Pyxis
{

	Entity::Entity(const std::string& name)
		: m_ID(++entityCounter)
	{

	}

	void Entity::OnUpdate(Timestep ts)
	{
		for (Ref<Entity> prototype : m_Children)
		{
			prototype->OnUpdate(ts);
		}
	}

	void Entity::AddChild(const Ref<Entity>& child)
	{
		m_Children.push_back(child);
	}

	void Entity::RemoveChild(const Ref<Entity>& child) 
	{
		for (auto it = m_Children.begin(); it != m_Children.end(); it++)
		{
			if (it->get() == child.get())
			{
				m_Children.erase(it);
				break;
			}
		}
	}

	void Entity::InspectorRender()
	{
		bool opened = ImGui::TreeNodeEx("Transform");
		if (opened)
		{
			
			
			glm::vec3 Position = m_Transform[3];
			glm::vec3 posCopy = glm::vec3(Position);
			ImGui::InputFloat3("Position", glm::value_ptr(Position));
			glm::vec3 diff = Position - posCopy;
			m_Transform = glm::translate(m_Transform, diff);
			
			ImGui::TreePop();
		}
		//ImGui::End();
	}


	EntityWithSprite::EntityWithSprite(const std::string& name) : Entity(name)
	{
		//Entity::Entity(name);
		m_Name = name;
		m_Transform = glm::mat4(1);
		m_Texture = nullptr;
	}
	EntityWithSprite::EntityWithSprite(Ref<Texture2D> texture, const std::string& name)
	{
		m_Transform = glm::mat4(1);
		m_Texture = texture;
	}
	void EntityWithSprite::OnUpdate(Timestep ts)
	{
		Entity::OnUpdate(ts);
		if (m_Texture)
		{
			Renderer2D::DrawQuad(m_Transform, m_Texture);
		}
		else
		{
			Renderer2D::DrawQuad(m_Transform, { 1,1,1,1 });
		}
	}

}
