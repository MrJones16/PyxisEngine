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
		class UIText : public UINode
		{
		public:

			std::string m_Text = "Text";
			glm::vec4 m_Color = glm::vec4(1);
			float m_MaxWidth = 10;
			float m_FontSize = 20;
			Ref<Font> m_Font;

			UIText(Ref<Font> font) : UINode("UIText"),
				m_Font(font)
			{

			}

			virtual ~UIText() = default;

			virtual void InspectorRender() override
			{
				UINode::InspectorRender();
				if (ImGui::TreeNodeEx("Text", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorEdit4("Color", glm::value_ptr(m_Color));
					ImGui::DragFloat("Max Width", &m_MaxWidth);
					ImGui::DragFloat("Font Size", &m_FontSize);

					ImGui::InputTextMultiline("##Text", &m_Text);
					//ImGui::InputText("##Text", &m_Text);

					ImGui::TreePop();
				}
			}

			//virtual void OnUpdate(Timestep ts) override;

			virtual void OnRender() override
			{
				///render children
				UINode::OnRender();

				Renderer2D::DrawText(m_Text, GetWorldTransform(), m_Font, m_FontSize, 1.3f, m_Color);
				
			}
		};

	}
}