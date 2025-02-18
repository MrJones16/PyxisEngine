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
		/// 
		/// Buttons that need to point to a function, that you want serialized, need to be
		/// member variables, and not children, and you should manually serialize & de-serialize
		/// in order to hook up the function pointer!
		/// </summary>
		class Button : public UIRect
		{
		protected:
			std::function<void()> m_Function = nullptr;
			bool m_Pressed = false;

		public:

			Ref<Texture2DResource> m_TextureResourcePressed = nullptr;

			Button(const std::string& name = "Button", const std::function<void()>& function = nullptr) : 
				UIRect(name), m_Function(function)
			{

			}

			Button(UUID id) : UIRect(id)
			{

			}

			Button(const std::string& name = "Button", Ref<Texture2DResource> texture = nullptr, const std::function<void()>& function = nullptr) : 
				UIRect(texture, name), m_Function(function)
			{

			}

			Button(const std::string& name = "Button", const glm::vec4& color = glm::vec4(1), const std::function<void()>& function = nullptr) :
				UIRect(color, name), m_Function(function)
			{

			}

			//Serialization
			virtual void Serialize(json& j) override
			{
				UIRect::Serialize(j);
				j["Type"] = "Button";

				//Add new member variables
				if (m_TextureResourcePressed != nullptr)
					j["m_TextureResourcePressed"] = m_TextureResourcePressed->GetPath();
			}
			virtual void Deserialize(json& j) override
			{
				UIRect::Deserialize(j);

				//Extract new member variables
				if (j.contains("m_TextureResourcePressed"))
				{
					std::string filepath = "";
					j.at("m_TextureResourcePressed").get_to(filepath);
					m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>(filepath);
				}
			}

			void SetFunction(const std::function<void()>& function)
			{
				m_Function = function;
			}

			virtual ~Button() = default;

			virtual void OnInspectorRender() override
			{
				UIRect::OnInspectorRender();
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
				}
				
			}
		};

		REGISTER_SERIALIZABLE_NODE(Button)

	}
}