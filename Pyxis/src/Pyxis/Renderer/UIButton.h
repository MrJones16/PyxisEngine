#pragma once
#include "UIRect.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that will display either the solid color given, or texture.
		/// </summary>
		class UIButton : public UIRect
		{
		public:

			/*Ref<Texture2D> m_Texture = nullptr;
			glm::vec4 m_Color = glm::vec4(1);
			glm::vec2 m_Size = glm::vec2(1);*/

			//overriding default color so easier to see
			glm::vec4 m_Color = glm::vec4(0.1f, 0.8f, 0.2f, 1);
			glm::vec2 m_Size = glm::vec2(1, 0.5f);

			UIButton(const std::string& name = "UIButton") : UIRect(name)
			{

			}

			UIButton(Ref<Texture2D> texture, const std::string& name = "UIButton") : UIRect(texture, name)
			{

			}

			UIButton(const glm::vec4& color, const std::string& name = "UIButton") : UIRect(color, name)
			{

			}

			virtual ~UIButton() = default;

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("Button", ImGuiTreeNodeFlags_DefaultOpen))
				{
					//Size
					ImGui::DragFloat2("Size", glm::value_ptr(m_Size));

					ImGui::TreePop();
				}
			}

			virtual void OnClick() override
			{
				PX_WARN("Mouse Pressed On Button!");
			}

			/*virtual void OnUpdate(Timestep ts)
			{

			}*/

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					if (m_Texture != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Texture->GetWidth(), m_Texture->GetHeight(), 1 });

						//TODO: Test ordering
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Texture, GetID());
					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, GetID());
					}
				}
				
			}
		};

	}
}