#pragma once

#include <Pyxis/Nodes/UI/UIRect.h>
#include <Pyxis/Events/EventSignals.h>
#include <Pyxis/Core/InputCodes.h>

namespace Pyxis
{
	namespace UI
	{

		//if you give it a pointer to a string, be sure to keep that string in memory while this is alive!
		class InputBase : public UIRect
		{
		protected:

			Reciever<void(KeyTypedEvent&)> m_KeyTypedReciever;
			Reciever<void(KeyPressedEvent&)> m_KeyPressedReciever;

			//Definitely could add a function pointer for when the user presses Enter
		public:

			Ref<Texture2DResource> m_TextureResourceSelected = nullptr;

			bool m_DisableRect = false;


		public:
			InputBase(const std::string& name) : UIRect(name),
				m_KeyTypedReciever(this, &InputBase::OnKeyTyped),
				m_KeyPressedReciever(this, &InputBase::OnKeyPressed)
			{

				//add our reciever to the key pressed signal
				EventSignal::s_KeyTypedEventSignal.AddReciever(m_KeyTypedReciever);
				EventSignal::s_KeyPressedEventSignal.AddReciever(m_KeyPressedReciever);
			}

			virtual ~InputBase()
			{
				
			}

			virtual void OnInspectorRender() override
			{
				UIRect::OnInspectorRender();
				ImGui::Checkbox("Disable Rect", &m_DisableRect);
			}

			virtual void OnKeyTyped(KeyTypedEvent& e)
			{
				
			}

			virtual void OnKeyPressed(KeyPressedEvent& e)
			{
				if (m_Enabled && s_MousePressedNodeID == m_UUID)
				{
					int keycode = e.GetKeyCode();

					if (keycode == PX_KEY_ENTER)
					{
						OnEnterPressed();
					}
				}
			}

			virtual void OnEnterPressed()
			{

			}

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
					if (!m_DisableRect)
					{
						if (m_TextureResource != nullptr)
						{
							if (m_TextureResourceSelected != nullptr && s_MousePressedNodeID == m_UUID)
							{
								Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResourceSelected->m_Texture, m_UUID, 1, m_Color);
							}
							else
							{
								Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, m_UUID, 1, m_Color);
							}
						}
						else
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, m_UUID);
						}
					}
				}
			}

		};
	}
}