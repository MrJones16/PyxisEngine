#pragma once
#include "UIButton.h"
#include "imgui_stdlib.h"
#include <Pyxis/Core/InputCodes.h>

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that functions as a button. can definitely be made into a templated if i need
		/// to have more complex arguments or return types?
		/// </summary>
		class TextButton : public UIButton
		{
		public:
			
			std::string m_Text = "Text";
			glm::vec4 m_TextColor = glm::vec4(0, 0, 0, 1);
			float m_FontSize = 20;
			Ref<Font> m_Font;
			bool m_ScaleText = true;

		public:

			TextButton(const std::string& name = "TextButton", Ref<Font> font = nullptr, const std::function<void()>& function = nullptr) :
				UIButton(name, function),
				m_Font(font)
			{

			}
			

			virtual ~TextButton() = default;

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("TextButton", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorEdit4("Color", glm::value_ptr(m_Color));

					ImGui::DragFloat("Font Size", &m_FontSize);
					ImGui::ColorEdit4("Text Color", glm::value_ptr(m_TextColor));
					ImGui::Checkbox("Scale Text", &m_ScaleText);

					ImGui::InputTextMultiline("##Text", &m_Text);
					//ImGui::InputText("##Text", &m_Text);

					ImGui::TreePop();
				}
			}

			virtual void OnMousePressed(int mouseButton) override
			{
				if (mouseButton == PX_MOUSE_BUTTON_1)
				{
					m_Pressed = true;
				}
			}

			virtual void OnMouseReleased(int mouseButton, bool continuous) override
			{
				if (mouseButton == PX_MOUSE_BUTTON_1)
				{
					m_Pressed = false;
					if (continuous && m_Function != nullptr)
					{
						m_Function();
					}
				}

			}

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					if (m_TextureResource != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering

						if (m_TexturePressedResource != nullptr && m_Pressed)
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TexturePressedResource->m_Texture, GetID(), 1, m_Color);
						}
						else
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, GetID(), 1, m_Color);
						}


					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, GetID());
					}

					
					float characterHeight = m_FontSize * m_Font->m_CharacterHeight * 1.3f;
					
					Renderer2D::DrawTextLine(m_Text, GetWorldTransform() * glm::translate(glm::mat4(1), { 0, -characterHeight / 2, 0 }), m_Font, m_FontSize, 1.3f, m_Size.x, UI::Left, m_ScaleText, m_TextColor, GetID());
				}

			}
		};

	}
}