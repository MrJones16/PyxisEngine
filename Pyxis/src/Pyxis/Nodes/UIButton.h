#pragma once
#include "UIRect.h"
#include <Pyxis/Core/InputCodes.h>

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that functions as a button. can definitely be made into a templated if i need
		/// to have more complex arguments or return types?
		/// </summary>
		class UIButton : public UIRect
		{
		private:
			std::function<void()> m_Function = nullptr;
			bool m_Pressed = false;

		public:

			Ref<Texture2DResource> m_TexturePressedResource = nullptr;

			UIButton(const std::string& name = "UIButton", const std::function<void()>& function = nullptr) : 
				UIRect(name), m_Function(function)
			{

			}

			UIButton(const std::string& name = "UIButton", Ref<Texture2DResource> texture = nullptr, const std::function<void()>& function = nullptr) : 
				UIRect(texture, name), m_Function(function)
			{

			}

			UIButton(const std::string& name = "UIButton", const glm::vec4& color = glm::vec4(1), const std::function<void()>& function = nullptr) :
				UIRect(color, name), m_Function(function)
			{

			}

			void SetFunction(const std::function<void()>& function)
			{
				m_Function = function;
			}

			virtual ~UIButton() = default;

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
					if (continuous && m_Function != nullptr)
					{
						m_Function();
					}
				}
				
			}

			/*virtual void OnUpdate(Timestep ts)
			{

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
				}
				
			}
		};

	}
}