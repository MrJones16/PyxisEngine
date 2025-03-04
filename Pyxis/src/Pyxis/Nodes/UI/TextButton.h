#pragma once
#include "Button.h"
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
		class TextButton : public Button
		{
		public:
			
			std::string m_Text = "Text";
			glm::vec4 m_TextColor = glm::vec4(0, 0, 0, 1);
			float m_FontSize = 1;
			Ref<Font> m_Font;
			bool m_ScaleText = true;
			glm::vec2 m_TextBorderSize = glm::vec2(0,0);
			glm::vec3 m_TextOffset = glm::vec3(0, 0, -0.0001f);
			glm::vec3 m_TextOffsetPressed = glm::vec3(0, 0, -0.0001f);

		public:

			TextButton(const std::string& name = "TextButton", Ref<Font> font = nullptr, const std::function<void()>& function = nullptr) :
				Button(name, function), m_Font(font)
			{

			}

			TextButton(UUID id) : Button(id)
			{
				m_Font = ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf");
			}			

			virtual ~TextButton() = default;

			//Serialize
			virtual void Serialize(json& j) override
			{
				Button::Serialize(j);
				j["Type"] = "TextButton";

				//Add new member variables
				if (m_Font != nullptr) 
					j["m_Font"] = m_Font->GetPath();
				j["m_Text"] = m_Text;
				j["m_FontSize"] = m_FontSize;
				j["m_TextColor"] = m_TextColor;
				j["m_ScaleText"] = m_ScaleText;
				j["m_TextBorderSize"] = m_TextBorderSize;
				j["m_TextOffset"] = m_TextOffset;
				j["m_TextOffsetPressed"] = m_TextOffsetPressed;
			}

			//Deserialize
			virtual void Deserialize(json& j) override
			{
				Button::Deserialize(j);

				if (j.contains("m_Font")) m_Font = ResourceManager::Load<Font>(j.at("m_Font").get<std::string>());
				if (j.contains("m_Text")) j.at("m_Text").get_to(m_Text);
				if (j.contains("m_FontSize")) j.at("m_FontSize").get_to(m_FontSize);
				if (j.contains("m_TextColor")) j.at("m_TextColor").get_to(m_TextColor);
				if (j.contains("m_ScaleText")) j.at("m_ScaleText").get_to(m_ScaleText);
				if (j.contains("m_TextBorderSize")) j.at("m_TextBorderSize").get_to(m_TextBorderSize);
				if (j.contains("m_TextOffset")) j.at("m_TextOffset").get_to(m_TextOffset);
				if (j.contains("m_TextOffsetPressed")) j.at("m_TextOffsetPressed").get_to(m_TextOffsetPressed);
			}


			virtual void OnInspectorRender() override
			{
				UIRect::OnInspectorRender();
				if (ImGui::TreeNodeEx("TextButton", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorEdit4("Color", glm::value_ptr(m_Color));

					ImGui::DragFloat("Font Size", &m_FontSize);
					ImGui::ColorEdit4("Text Color", glm::value_ptr(m_TextColor));
					ImGui::Checkbox("Scale Text", &m_ScaleText);
					ImGui::InputFloat2("Text Border Size", glm::value_ptr(m_TextBorderSize));

					ImGui::InputFloat3("Text Offset", glm::value_ptr(m_TextOffset));
					ImGui::InputFloat3("Text Pressed Offset", glm::value_ptr(m_TextOffsetPressed));

					ImGui::InputTextMultiline("##Text", &m_Text);
					//ImGui::InputText("##Text", &m_Text);

					ImGui::TreePop();
				}
			}

			/*virtual void OnMousePressed(int mouseButton) override
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

			}*/

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					if (m_TextureResource != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering

						if (m_TextureResourcePressed != nullptr && m_Pressed)
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResourcePressed->m_Texture, GetUUID(), 1, m_Color);
						}
						else
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, GetUUID(), 1, m_Color);
						}
					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, GetUUID());
					}

					
					float characterHeight = m_FontSize * m_Font->m_CharacterHeight * 1.3f;
					// -characterHeight / 2
					glm::vec2 maxSize = m_Size - (m_TextBorderSize * (2.0f / m_PPU));
					glm::vec3 offset = (m_Pressed ? m_TextOffsetPressed : m_TextOffset) * (1.0f/m_PPU);
					Renderer2D::DrawTextLine(m_Text, GetWorldTransform() * glm::translate(glm::mat4(1), offset), m_Font, maxSize, m_FontSize, UI::Center, m_ScaleText, m_TextColor, GetUUID());
				}

			}
		};
		REGISTER_SERIALIZABLE_NODE(TextButton);

	}
}