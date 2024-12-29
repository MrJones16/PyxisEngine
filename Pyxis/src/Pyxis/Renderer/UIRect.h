#pragma once
#include "UINode.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that will display either the solid color given, or texture.
		/// </summary>
		class UIRect : public UINode
		{
		public:

			Ref<Texture2D> m_Texture = nullptr;
			glm::vec4 m_Color = glm::vec4(1);
			glm::vec2 m_Size = glm::vec2(1);

			UIRect(const std::string& name = "UIRect") : UINode(name) 
			{
			}
			UIRect(Ref<Texture2D> texture, const std::string& name = "UIRect") : UINode(name),
				m_Texture(texture), m_Color(1)
			{

			}
			UIRect(const glm::vec4& color, const std::string& name = "UIRect") : UINode(name),
				m_Texture(nullptr), m_Color(color)
			{

			}
			virtual ~UIRect() = default;

			virtual void InspectorRender() override
			{
				UINode::InspectorRender();
				if (ImGui::TreeNodeEx("Rect"))
				{
					//Size
					ImGui::DragFloat2("Size", glm::value_ptr(m_Size));

					ImGui::TreePop();
				}
			}

			//virtual void OnUpdate(Timestep ts) override;

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					UINode::OnRender();
					if (m_Texture != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Texture->GetWidth(), m_Texture->GetHeight(), 1 });

						//TODO: Test ordering
						Renderer2D::DrawQuad(GetWorldTransform() * sizeMat, m_Texture);
					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuad(GetWorldTransform() * sizeMat, m_Color);
					}
				}
				
			}
		};

	}
}