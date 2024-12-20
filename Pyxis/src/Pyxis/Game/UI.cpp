#include "pxpch.h"

#include "UI.h"

namespace Pyxis
{
	UIRect::UIRect(Ref<Texture2D> texture, const std::string& name) : UINode(name), 
		m_Texture(texture), m_Color(1)
	{

	}

	UIRect::UIRect(const glm::vec4& color, const std::string& name) : UINode(name),
		m_Texture(nullptr), m_Color(color)
	{
		
	}

	void UIRect::InspectorRender()
	{
		bool opened = ImGui::TreeNodeEx("Transform");
		if (opened)
		{

			// Position Editing
			glm::vec3 Position = m_LocalTransform[3];
			glm::vec3 posCopy = glm::vec3(Position);
			ImGui::InputFloat3("Position", glm::value_ptr(Position));
			glm::vec3 diff = Position - posCopy;
			m_LocalTransform = glm::translate(m_LocalTransform, diff);

			//Size
			ImGui::DragFloat2("Size", glm::value_ptr(m_Size));


			ImGui::TreePop();
		}
	}

	void UIRect::OnRender()
	{
		UINode::OnRender();
		if (m_Texture != nullptr)
		{
			//we have a texture, so display it!
			glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Texture->GetWidth(), m_Texture->GetHeight(), 1});
			
			//TODO: Test ordering
			Renderer2D::DrawQuad(GetTransform() * sizeMat, m_Texture);
		}
		else
		{
			//just draw the color as the square
			glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
			Renderer2D::DrawQuad(GetTransform() * sizeMat, m_Color);
		}
	}
}