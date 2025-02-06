#pragma once
#include "UINode.h"
#include "imgui_stdlib.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that will display either the solid color given, or texture.
		/// </summary>
		class UIText : public UIRect
		{
		public:

			std::string m_Text = "Text";
			float m_FontSize = 20;
			Ref<Font> m_Font;
			bool m_ShowRegion = false;
			bool m_Centered = false;

			//default: Left. Only Left, Center, and ___ work
			UI::Direction m_TextAlignment = Left;
			

			UIText(Ref<Font> font) : UIRect("UIText"),
				m_Font(font)
			{
				m_Color = { 0,0,0,1 };
			}

			virtual ~UIText() = default;

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("Text", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorEdit4("Color", glm::value_ptr(m_Color));
					ImGui::DragFloat("Font Size", &m_FontSize);

					ImGui::Checkbox("Show Region", &m_ShowRegion);

					ImGui::Checkbox("Centered", &m_Centered);

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
					uint32_t nodeID = m_Parent ? m_Parent->GetID() : GetID();
					float characterHeight = m_FontSize * m_Font->m_CharacterHeight * 1.3f;
					if (m_Centered)
					{
						Renderer2D::DrawTextLine(m_Text, GetWorldTransform() * glm::translate(glm::mat4(1), { 0, -characterHeight/2, 0 }), m_Font, m_Size, m_FontSize, UI::Left, false, m_Color, nodeID);
					}
					else
					{
						Renderer2D::DrawText(m_Text, GetWorldTransform() * glm::translate(glm::mat4(1), {-m_Size.x / 2, (m_Size.y / 2) - characterHeight, 0}), m_Font, m_FontSize, 1.3f, m_Size.x, UI::Left, m_Color, nodeID);
					}

					glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
					if (m_ShowRegion)
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, { 0,0,1, 0.2f }, nodeID);
				}
			}
		};

	}
}