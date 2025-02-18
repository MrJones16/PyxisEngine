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
		class SignalButton : public UIRect
		{
		private:
			Signal<void()> m_Signal;
			bool m_Pressed = false;

		public:

			Ref<Texture2DResource> m_TexturePressedResource = nullptr;
			//overriding default color so easier to see
			/*glm::vec4 m_Color = glm::vec4(0.1f, 0.8f, 0.2f, 1);
			glm::vec2 m_Size = glm::vec2(1, 0.5f);*/

			SignalButton(const std::string& name = "SignalButton") : UIRect(name)
			{

			}

			SignalButton(Ref<Texture2DResource> texture, const std::string& name = "SignalButton") : UIRect(texture, name)
			{

			}

			SignalButton(const glm::vec4& color, const std::string& name = "SignalButton") : UIRect(color, name)
			{

			}

			SignalButton(UUID id) : UIRect(id)
			{

			}

			virtual ~SignalButton() = default;

			void AddReciever(const Reciever<void()>& reciever)
			{
				m_Signal.AddReciever(reciever);
			}

			//Serialize
			virtual void Serialize(json& j) override
			{
				UIRect::Serialize(j);
				j["Type"] = "SignalButton";

				//Add new member variables
				if (m_TexturePressedResource != nullptr)
					j["m_TexturePressedResource"] = m_TexturePressedResource->GetPath();
			}

			//Deserialize
			virtual void Deserialize(json& j) override
			{
				UIRect::Deserialize(j);
				//Extract new member variables
				if (j.contains("m_TexturePressedResource")) m_TexturePressedResource = 
					ResourceManager::Load<Texture2DResource>(j.at("m_TexturePressedResource").get<std::string>());
			}

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
					if (m_TextureResource != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering

						if (m_TexturePressedResource != nullptr && m_Pressed)
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TexturePressedResource->m_Texture, GetUUID());
						}
						else
						{
							Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, GetUUID());
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
		REGISTER_SERIALIZABLE_NODE(SignalButton);

	}
}