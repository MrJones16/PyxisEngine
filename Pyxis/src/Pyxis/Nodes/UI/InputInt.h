#pragma once

#include "InputText.h"

namespace Pyxis
{
	namespace UI
	{

		class InputInt : public InputText
		{
		public:
			int m_MinValue = INT_MIN;
			int m_MaxValue = INT_MAX;

			int* m_Value;


		public:
			InputInt(const std::string& name, Ref<Font> font, int* value) : InputText(name, font), m_Value(value)
			{
				m_Text = std::to_string(*value);
			}

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("Int Edit", ImGuiTreeNodeFlags_DefaultOpen))
				{
					
					ImGui::InputInt("Min Value", &m_MinValue);
					ImGui::InputInt("Max Value", &m_MaxValue);


					ImGui::TreePop();
				}
			}

			virtual void OnKeyTyped(KeyTypedEvent& e) override
			{
				if (m_Enabled && s_MousePressedNodeID == m_ID)
				{
					int keycode = e.GetKeyCode();

					m_Text.push_back(keycode);
					int newValue = std::max(std::min((int)std::atoll(m_Text.c_str()), m_MaxValue), m_MinValue);
					m_Text = std::to_string(newValue);
					*m_Value = newValue;
				}

			}

			virtual void OnKeyPressed(KeyPressedEvent& e) override
			{
				if (m_Enabled && s_MousePressedNodeID == m_ID)
				{
					int keycode = e.GetKeyCode();

					if (keycode == PX_KEY_BACKSPACE)
					{
						if (m_Text.size() > 0)
							m_Text.pop_back();
					}

					int newValue = std::max(std::min((int)std::atoll(m_Text.c_str()), m_MaxValue), m_MinValue);

					if (keycode == PX_KEY_MINUS)
					{
						newValue *= -1;
					}
					m_Text = std::to_string(newValue);
					*m_Value = newValue;
				}

			}

		};
	}
}
