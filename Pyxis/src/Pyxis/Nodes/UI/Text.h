#pragma once
#include "UIRect.h"
#include "imgui_stdlib.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that will display either the solid color given, or texture.
		/// </summary>
		class Text : public UIRect
		{
		public:

			std::string m_Text = "Text";
			glm::vec4 m_TextColor = glm::vec4(0,0,0,1);
			glm::vec2 m_TextBorderSize = glm::vec2(0, 0);
			float m_FontSize = 20;
			Ref<Font> m_Font;
			bool m_RenderRect = false;
			bool m_MultiLine = true;
			/// <summary>
			/// only works if multiline is false! (atm?)
			/// </summary>
			bool m_ScaleToFit = true;
			UI::Direction m_Alignment = UI::Direction::Left;
			bool m_SelectParentID = true;

			//default: Left. Only Left, Center, and (right soon) work
			UI::Direction m_TextAlignment = Left;
			
			Text(const std::string& name, Ref<Font> font) : UIRect(name),
				m_Font(font)
			{				
				
			}

			Text(Ref<Font> font) : UIRect("Text"),
				m_Font(font)
			{
				
			}

			virtual ~Text() = default;

			virtual void OnInspectorRender() override
			{
				UIRect::OnInspectorRender();
				if (ImGui::TreeNodeEx("Text", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorEdit4("Text Color", glm::value_ptr(m_TextColor));
					ImGui::DragFloat("Font Size", &m_FontSize);

					ImGui::Checkbox("Show Region", &m_RenderRect);

					ImGui::InputFloat2("Text Border Size", glm::value_ptr(m_TextBorderSize));

					if (ImGui::TreeNodeEx("Alignment", ImGuiTreeNodeFlags_DefaultOpen))
					{

						if (ImGui::Button("Left")) { m_Alignment = Direction::Left; }
						if (ImGui::Button("Centered")) { m_Alignment = Direction::Center; }
						//if (ImGui::Button("Right")) { m_Alignment = Direction::Right; }
						ImGui::TreePop();
					}

					ImGui::InputTextMultiline("##Text", &m_Text);
					//ImGui::InputText("##Text", &m_Text);

					ImGui::TreePop();
				}
			}

			//virtual void OnUpdate(Timestep ts) override;

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					uint32_t nodeID = (m_Parent && m_SelectParentID) ? m_Parent->GetUUID() : GetUUID();

					glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
					if (m_RenderRect)
					{
						if (m_TextureResource != nullptr)
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, nodeID, 1, m_Color);
						}
						else
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, nodeID);
						}

					}


					float characterHeight = m_FontSize * m_Font->m_CharacterHeight * 1.3f;

					glm::vec2 maxSize = m_Size - (m_TextBorderSize * (2.0f / m_PPU));
					if (m_MultiLine)
					{
						Renderer2D::DrawText(m_Text, GetWorldTransform() * glm::translate(glm::mat4(1), {-m_Size.x / 2, (m_Size.y / 2) - characterHeight, 0}), m_Font, m_FontSize, 1.3f, m_Size.x - m_TextBorderSize.x, m_Alignment, m_TextColor, nodeID);
					}
					else
					{
						Renderer2D::DrawTextLine(m_Text, GetWorldTransform() * glm::translate(glm::mat4(1), { 0, 0, 0 }), m_Font, m_Size - (m_TextBorderSize * 2.0f), m_FontSize, m_Alignment, m_ScaleToFit, m_TextColor, nodeID);
					}

					
				}
			}
		};

	}
}