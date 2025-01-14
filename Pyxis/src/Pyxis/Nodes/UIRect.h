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
			glm::vec2 m_Size = { 1,1 };
			float m_PPU = 32;

			bool m_AutoResizing = false;
			glm::vec2 m_AutomaticSizingPercent = { 1,1 };

			bool m_AutoAlign = false;
			Direction m_HorizontalAlignment = Center;
			Direction m_VerticalAlignment = Center;

			UIRect(const std::string& name = "UIRect") : UINode(name) 
			{

			}

			UIRect(Ref<Texture2D> texture, const std::string& name = "UIRect") : UINode(name),
				m_Texture(texture), m_Color(1)
			{
				UpdateSizeFromTexture();
			}

			UIRect(const glm::vec4& color, const std::string& name = "UIRect") : UINode(name),
				m_Texture(nullptr), m_Color(color)
			{

			}



			//uses the PPU and the texture to set the size of the object
			void UpdateSizeFromTexture()
			{
				if (m_Texture != nullptr)
				{
					m_Size = { (float)m_Texture->GetWidth() / m_PPU , (float)m_Texture->GetHeight() / m_PPU };
				}
			}

			virtual ~UIRect() = default;

			virtual void InspectorRender() override
			{
				UINode::InspectorRender();
				if (ImGui::TreeNodeEx("Rect", ImGuiTreeNodeFlags_DefaultOpen))
				{
					//Size
					ImGui::DragFloat2("Size", glm::value_ptr(m_Size));

					ImGui::ColorEdit4("Color", glm::value_ptr(m_Color));

					ImGui::TreePop();
				}
			}

			void AutoRect()
			{
				if (auto parentRect = dynamic_cast<UIRect*>(m_Parent))
				{
					if (m_AutoResizing)
						m_Size = parentRect->m_Size * m_AutomaticSizingPercent;

					if (m_AutoAlign)
					{
						//reset position
						m_Position = glm::vec3(0, 0, m_Position.z);
						//horizontal
						if (m_HorizontalAlignment == Left)
						{
							m_Position.x -= (parentRect->m_Size.x / 2.0f) - (m_Size.x / 2);
						}
						else if (m_HorizontalAlignment == Right)
						{
							m_Position.x += (parentRect->m_Size.x / 2.0f) - (m_Size.x / 2);
						}

						//vertical
						if (m_VerticalAlignment == Up)
						{
							m_Position.y += (parentRect->m_Size.y / 2.0f) - (m_Size.y / 2);
						}
						else if (m_VerticalAlignment == Down)
						{
							m_Position.y -= (parentRect->m_Size.y / 2.0f) - (m_Size.y / 2);
						}
						
						UpdateLocalTransform();
					}
				}
				
			}

			virtual void PropagateUpdate() override
			{
				AutoRect();
				UINode::PropagateUpdate();
			}

			//virtual void OnUpdate(Timestep ts) override;

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					if (m_Texture != nullptr)
					{	
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Texture, GetID(), 1, m_Color);
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