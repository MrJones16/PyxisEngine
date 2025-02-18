#pragma once

#include <Pyxis/Nodes/UI/InputBase.h>

namespace Pyxis
{
	namespace UI
	{

		

		/// <summary>
		/// Needs to be serialized separately from children if you want
		/// to hook up the value's pointer
		/// 
		/// if you give it a pointer to a string, be sure to keep that
		/// string in memory while this is alive
		/// </summary>
		class InputText : public InputBase
		{
		protected:

			//to track if we have created our own value (in this case string) so we know to free the memory upon deletion
			bool m_OwnsValue = false;
			std::string* m_Value;
			
		public:
			
			//Text Display Variables
			glm::vec4 m_TextColor = glm::vec4(0, 0, 0, 1);
			glm::vec2 m_TextBorderSize = glm::vec2(0, 0);
			float m_FontSize = 1;
			Ref<Font> m_Font;
			bool m_RenderRect = false;
			bool m_Multiline = true;
			/// <summary>
			/// only works if multiline is false! (atm?)
			/// </summary>
			bool m_ScaleToFit = true;
			UI::Direction m_Alignment = UI::Direction::Left;


		public:
			InputText(const std::string& name, Ref<Font> font, std::string* value = nullptr) : InputBase(name),
				m_Font(font)
			{
				//standard input text settings
				m_Multiline = false;
				m_Alignment = Direction::Left;
				m_RenderRect = true;

				if (value == nullptr)
				{
					m_Value = new std::string();
					m_OwnsValue = true;
				}
				else
				{
					m_Value = value;
				}
			}

			InputText(UUID id) : InputBase(id)
			{
				//standard input text settings
				m_Multiline = false;
				m_Alignment = Direction::Left;
				m_RenderRect = true;

				m_Font = ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf");
				m_Value = new std::string();
				m_OwnsValue = true;
				
			}

			virtual ~InputText()
			{
				if (m_OwnsValue) delete m_Value;
			}

			virtual void SetValue(std::string* s)
			{
				if (s == nullptr) return;
				if (m_OwnsValue) delete m_Value;
				m_Value = s;
				m_OwnsValue = false;
			}

			//Serialize
			virtual void Serialize(json& j) override
			{
				InputBase::Serialize(j);
				j["Type"] = "InputText";

				//Add new member variables
				j["m_TextColor"] = m_TextColor;
				j["m_TextBorderSize"] = m_TextBorderSize;
				j["m_Font"] = m_Font->GetPath();
				j["m_FontSize"] = m_FontSize;
				j["m_RenderRect"] = m_RenderRect;
				j["m_Multiline"] = m_Multiline;
				j["m_ScaleToFit"] = m_ScaleToFit;
				j["m_Alignment"] = m_Alignment;
			}
			//Deserialize
			virtual void Deserialize(json& j) override
			{
				InputBase::Deserialize(j);

				//Extract new member variables
				if (j.contains("m_TextColor")) j.at("m_TextColor").get_to(m_TextColor);
				if (j.contains("m_TextBorderSize")) j.at("m_TextBorderSize").get_to(m_TextBorderSize);
				if (j.contains("m_Font")) m_Font = ResourceManager::Load<Font>(j.at("m_Font").get<std::string>());
				if (j.contains("m_FontSize")) j.at("m_FontSize").get_to(m_FontSize);
				if (j.contains("m_RenderRect")) j.at("m_RenderRect").get_to(m_RenderRect);
				if (j.contains("m_Multiline")) j.at("m_Multiline").get_to(m_Multiline);
				if (j.contains("m_ScaleToFit")) j.at("m_ScaleToFit").get_to(m_ScaleToFit);
				if (j.contains("m_Alignment")) j.at("m_Alignment").get_to(m_Alignment);
			}

			virtual void OnInspectorRender() override
			{
				InputBase::OnInspectorRender();
				if (ImGui::TreeNodeEx("Text Input", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::ColorEdit4("Text Color", glm::value_ptr(m_TextColor));
					ImGui::DragFloat("Font Size", &m_FontSize);
					ImGui::Checkbox("Multiline", &m_Multiline);					

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
				if (m_Enabled && s_MousePressedNodeID == m_UUID)
				{
					int keycode = e.GetKeyCode();
				
					(*m_Value).push_back(keycode);
				}
					
			}

			virtual void OnKeyPressed(KeyPressedEvent& e)
			{
				if (m_Enabled && s_MousePressedNodeID == m_UUID)
				{
					int keycode = e.GetKeyCode();

					if (keycode == PX_KEY_BACKSPACE)
					{
						if ((*m_Value).size() > 0)
							(*m_Value).pop_back();
					}
				}

			}

			virtual void OnRender() override
			{
				InputBase::OnRender();

				if (m_Enabled)
				{					

					float characterHeight = m_FontSize * m_Font->m_CharacterHeight * 1.3f;

					glm::vec2 maxSize = m_Size - (m_TextBorderSize * (2.0f / m_PPU));
					if (m_Multiline)
					{
						Renderer2D::DrawText((*m_Value), GetWorldTransform() * glm::translate(glm::mat4(1), { -m_Size.x / 2, (m_Size.y / 2) - characterHeight, -0.0001f }), m_Font, m_FontSize, 1.3f, m_Size.x - m_TextBorderSize.x, m_Alignment, m_TextColor, m_UUID);
					}
					else
					{
						//see if we want to add an input indicator
						auto time = std::chrono::high_resolution_clock::now();
						long long ms = std::chrono::time_point_cast<std::chrono::milliseconds>(time).time_since_epoch().count();

						const char* indicator = (s_MousePressedNodeID == m_UUID) && (ms / 500) % 2 == 0 ? "|" : "";
						
						Renderer2D::DrawTextLine((*m_Value) + indicator, GetWorldTransform() * glm::translate(glm::mat4(1), {0, 0, -0.0001f }), m_Font, maxSize, m_FontSize, m_Alignment, m_ScaleToFit, m_TextColor, m_UUID);
					}
				}
			}

		};
		REGISTER_SERIALIZABLE_NODE(InputText);
	}
}