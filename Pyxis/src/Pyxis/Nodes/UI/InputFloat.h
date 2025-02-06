#pragma once

#include "InputText.h"

namespace Pyxis
{
	namespace UI
	{

		class InputFloat : public InputText
		{
		public:
			float m_MinValue = FLT_MIN;
			float m_MaxValue = FLT_MAX;

			float* m_Value;


		public:
			InputFloat(const std::string& name, Ref<Font> font, float* value) : InputText(name, font), m_Value(value)
			{
				m_Text = std::to_string(*value);
			}

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("Float Edit", ImGuiTreeNodeFlags_DefaultOpen))
				{

					ImGui::InputFloat("Min Value", &m_MinValue);
					ImGui::InputFloat("Max Value", &m_MaxValue);


					ImGui::TreePop();
				}
			}

			virtual void OnKeyTyped(KeyTypedEvent& e) override
			{
				if (m_Enabled && s_MousePressedNodeID == m_ID)
				{
					int keycode = e.GetKeyCode();

					m_Text.push_back(keycode);
					float newValue = std::max(std::min(std::atof(m_Text.c_str()), (double)m_MaxValue), (double)m_MinValue);
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

					float newValue = std::max(std::min(std::atof(m_Text.c_str()), (double)m_MaxValue), (double)m_MinValue);

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
