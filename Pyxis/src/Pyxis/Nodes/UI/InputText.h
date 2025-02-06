#pragma once

#include <Pyxis/Nodes/UI/UIRect.h>
#include <Pyxis/Events/EventSignals.h>
#include <Pyxis/Core/InputCodes.h>

namespace Pyxis
{
	namespace UI
	{
		class InputText : public UIRect
		{
		protected:

			Reciever<void(KeyTypedEvent&)> m_KeyTypedReciever;
			Reciever<void(KeyPressedEvent&)> m_KeyPressedReciever;

			//Definitely could add a function pointer for when the user presses Enter
		public:

			std::string m_Text = "Text";
			glm::vec4 m_TextColor = glm::vec4(0, 0, 0, 1);
			glm::vec2 m_TextBorderSize = glm::vec2(0, 0);
			float m_FontSize = 20;
			Ref<Font> m_Font;
			bool m_RenderRect = false;
			bool m_Multiline = true;
			/// <summary>
			/// only works if multiline is false! (atm?)
			/// </summary>
			bool m_ScaleToFit = true;
			UI::Direction m_Alignment = UI::Direction::Left;

			Ref<Texture2DResource> m_TextureResourceSelected = nullptr;
		public:
			InputText(const std::string& name, Ref<Font> font) : UIRect(name),
				m_Font(font),
				m_KeyTypedReciever(this, &InputText::OnKeyTyped),
				m_KeyPressedReciever(this, &InputText::OnKeyPressed)
			{
				//standard input text settings
				m_Multiline = false;
				m_Alignment = Direction::Left;
				m_RenderRect = true;

				//add our reciever to the key pressed signal
				EventSignal::s_KeyTypedEventSignal.AddReciever(m_KeyTypedReciever);
				EventSignal::s_KeyPressedEventSignal.AddReciever(m_KeyPressedReciever);
			}

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("Text Input", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorEdit4("Text Color", glm::value_ptr(m_TextColor));
					ImGui::DragFloat("Font Size", &m_FontSize);
					ImGui::Checkbox("Multiline", &m_Multiline);

					ImGui::Checkbox("Show Region", &m_RenderRect);

					ImGui::InputFloat2("Text Border Size", glm::value_ptr(m_TextBorderSize));

					ImGui::Checkbox("Scale To Fit", &m_ScaleToFit);

					if (ImGui::TreeNodeEx("Alignment", ImGuiTreeNodeFlags_DefaultOpen))
					{

						if (ImGui::Button("Left")) { m_Alignment = Direction::Left; }
						if (ImGui::Button("Centered")) { m_Alignment = Direction::Center; }
						//if (ImGui::Button("Right")) { m_Alignment = Direction::Right; }
						ImGui::TreePop();
					}


					ImGui::TreePop();
				}
			}

			virtual void OnKeyTyped(KeyTypedEvent& e)
			{
				if (m_Enabled && s_MousePressedNodeID == m_ID)
				{
					int keycode = e.GetKeyCode();
				
					m_Text.push_back(keycode);
				}
					
			}

			virtual void OnKeyPressed(KeyPressedEvent& e)
			{
				if (m_Enabled && s_MousePressedNodeID == m_ID)
				{
					int keycode = e.GetKeyCode();

					if (keycode == PX_KEY_BACKSPACE)
					{
						if (m_Text.size() > 0)
							m_Text.pop_back();
					}
				}

			}

			virtual void OnRender() override
			{
				if (m_Enabled)
				{					

					glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
					if (m_RenderRect)
					{
						if (m_TextureResource != nullptr)
						{
							if (m_TextureResourceSelected != nullptr && s_MousePressedNodeID == m_ID)
							{
								Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResourceSelected->m_Texture, m_ID, 1, m_Color);
							}
							else
							{
								Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, m_ID, 1, m_Color);
							}
						}
						else
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, m_ID);
						}

					}


					float characterHeight = m_FontSize * m_Font->m_CharacterHeight * 1.3f;

					glm::vec2 maxSize = m_Size - (m_TextBorderSize * (2.0f / m_PPU));
					if (m_Multiline)
					{
						Renderer2D::DrawText(m_Text, GetWorldTransform() * glm::translate(glm::mat4(1), { -m_Size.x / 2, (m_Size.y / 2) - characterHeight, 0 }), m_Font, m_FontSize, 1.3f, m_Size.x - m_TextBorderSize.x, m_Alignment, m_TextColor, m_ID);
					}
					else
					{
						//see if we want to add an input indicator
						auto time = std::chrono::high_resolution_clock::now();
						long long ms = std::chrono::time_point_cast<std::chrono::milliseconds>(time).time_since_epoch().count();

						const char* indicator = (s_MousePressedNodeID == m_ID) && (ms / 500) % 2 == 0 ? "|" : "";
						
						Renderer2D::DrawTextLine(m_Text + indicator, GetWorldTransform() * glm::translate(glm::mat4(1), {0, 0, 0}), m_Font, maxSize, m_FontSize, m_Alignment, m_ScaleToFit, m_TextColor, m_ID);
					}


				}
			}

		};
	}
}