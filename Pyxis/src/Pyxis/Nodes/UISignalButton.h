#pragma once
#include "UIRect.h"
#include <Pyxis/Core/InputCodes.h>
#include <Pyxis/Events/Signal.h>

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that functions as a button. can definitely be made into a templated if i need
		/// to have more complex arguments or return types?
		/// </summary>
		class UISignalButton : public UIRect
		{
		private:
			Signal<void()> m_Signal;
			bool m_Pressed = false;

		public:

			Ref<Texture2D> m_TexturePressed = nullptr;
			//overriding default color so easier to see
			/*glm::vec4 m_Color = glm::vec4(0.1f, 0.8f, 0.2f, 1);
			glm::vec2 m_Size = glm::vec2(1, 0.5f);*/

			UISignalButton(const std::string& name = "UISignalButton") : UIRect(name)
			{

			}

			UISignalButton(Ref<Texture2D> texture, const std::string& name = "UISignalButton") : UIRect(texture, name)
			{

			}

			UISignalButton(const glm::vec4& color, const std::string& name = "UISignalButton") : UIRect(color, name)
			{

			}

			void AddReciever(const Reciever<void()>& reciever)
			{
				m_Signal.AddReciever(reciever);
			}

			virtual ~UISignalButton() = default;

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("Button", ImGuiTreeNodeFlags_DefaultOpen))
				{
					//Size
					ImGui::DragFloat2("Size", glm::value_ptr(m_Size));

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
					if (continuous) m_Signal();
				}
				
			}

			/*virtual void OnUpdate(Timestep ts)
			{

			}*/

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					if (m_Texture != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering

						if (m_TexturePressed != nullptr && m_Pressed)
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TexturePressed, GetID());
						}
						else
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Texture, GetID());
						}
						
						
					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, GetID());
					}
				}
				
			}
		};

	}
}