#pragma once
#include "UINode.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that will display either the solid color given, or texture.
		/// </summary>
		class UIRect : public UINode
		{
		public:

			Ref<Texture2DResource> m_TextureResource = nullptr;
			glm::vec4 m_Color = glm::vec4(1);
			glm::vec2 m_Size = { 1,1 };
			float m_PPU = 32;

			//automatic sizing
			bool m_AutomaticSizing = false;
			glm::vec2 m_AutomaticSizingPercent = { 1, 1 };
			glm::vec2 m_AutomaticSizingOffset = { 0, 0 };
			//-1 means not fixed, any other value will be set after sizing
			glm::vec2 m_FixedSize = { -1, -1 };

			///automatic positioning
			bool m_AutomaticPositioning = false;
			glm::vec3 m_AutomaticPositionOffset = { 0, 0, 0 };
			Direction m_HorizontalAlignment = Center;
			Direction m_VerticalAlignment = Center;

			UIRect(const std::string& name = "UIRect") : UINode(name) 
			{
				
			}

			UIRect(UUID id) : UINode(id)
			{

			}

			UIRect(Ref<Texture2DResource> texture, const std::string& name = "UIRect") : UINode(name),
				m_TextureResource(texture), m_Color(1)
			{
				UpdateSizeFromTexture();
			}

			UIRect(const glm::vec4& color, const std::string& name = "UIRect") : UINode(name),
				m_TextureResource(nullptr), m_Color(color)
			{

			}

			//Serialization
			virtual void Serialize(json& j) override
			{
				UINode::Serialize(j);
				j["Type"] = "UIRect";

				//Add new member variables
				if (m_TextureResource != nullptr)
					j["m_TextureResource"] = m_TextureResource->GetPath();
				j["m_Color"] = m_Color;
				j["m_Size"] = m_Size;
				j["m_PPU"] = m_PPU;

				j["m_AutomaticSizing"] = m_AutomaticSizing;
				j["m_AutomaticSizingPercent"] = m_AutomaticSizingPercent;
				j["m_AutomaticSizingOffset"] = m_AutomaticSizingOffset;
				j["m_FixedSize"] = m_FixedSize;

				j["m_AutomaticPositioning"] = m_AutomaticPositioning;
				j["m_AutomaticPositionOffset"] = m_AutomaticPositionOffset;
				j["m_HorizontalAlignment"] = m_HorizontalAlignment;
				j["m_VerticalAlignment"] = m_VerticalAlignment;
			}

			virtual void Deserialize(json& j) override
			{
				UINode::Deserialize(j);

				//Extract new member variables
				
				if (j.contains("m_TextureResource")) m_TextureResource = 
					ResourceManager::Load<Texture2DResource>(j.at("m_TextureResource").get<std::string>());				
				if (j.contains("m_Color")) j.at("m_Color").get_to(m_Color);
				if (j.contains("m_Size")) j.at("m_Size").get_to(m_Size);
				if (j.contains("m_PPU"))
				{
					j.at("m_PPU").get_to(m_PPU);
					UpdateSizeFromTexture();
				}

				if (j.contains("m_AutomaticSizing")) j.at("m_AutomaticSizing").get_to(m_AutomaticSizing);
				if (j.contains("m_AutomaticSizingPercent")) j.at("m_AutomaticSizingPercent").get_to(m_AutomaticSizingPercent);
				if (j.contains("m_AutomaticSizingOffset")) j.at("m_AutomaticSizingOffset").get_to(m_AutomaticSizingOffset);
				if (j.contains("m_FixedSize")) j.at("m_FixedSize").get_to(m_FixedSize);

				if (j.contains("m_AutomaticPositioning")) j.at("m_AutomaticPositioning").get_to(m_AutomaticPositioning);
				if (j.contains("m_AutomaticPositionOffset")) j.at("m_AutomaticPositionOffset").get_to(m_AutomaticPositionOffset);
				if (j.contains("m_HorizontalAlignment")) j.at("m_HorizontalAlignment").get_to(m_HorizontalAlignment);
				if (j.contains("m_VerticalAlignment")) j.at("m_VerticalAlignment").get_to(m_VerticalAlignment);
			}



			//uses the PPU and the texture to set the size of the object
			void UpdateSizeFromTexture()
			{
				if (m_TextureResource != nullptr)
				{
					m_Size = { (float)m_TextureResource->m_Texture->GetWidth() / m_PPU , (float)m_TextureResource->m_Texture->GetHeight() / m_PPU };
				}
			}

			virtual ~UIRect() = default;

			virtual void OnInspectorRender() override
			{
				UINode::OnInspectorRender();
				if (ImGui::TreeNodeEx("Rect", ImGuiTreeNodeFlags_DefaultOpen))
				{
					
					ImGui::DragFloat2("Size", glm::value_ptr(m_Size));

					ImGui::ColorEdit4("Color", glm::value_ptr(m_Color));

					if (ImGui::InputFloat("PPU", &m_PPU))
					{
						if (m_TextureResource != nullptr) UpdateSizeFromTexture();
					}
					ImGui::SetItemTooltip("Pixels Per Unit");

					if (ImGui::Button("Auto Rect")) AutoRect();


					if (ImGui::TreeNodeEx("Automatic Sizing", ImGuiTreeNodeFlags_DefaultOpen))
					{					

						ImGui::Checkbox("Automatic Sizing", &m_AutomaticSizing);

						ImGui::DragFloat2("Automatic Sizing Percent", glm::value_ptr(m_AutomaticSizingPercent), 0.1f, 0.0f, 1.0f);

						ImGui::InputFloat2("Automatic Sizing Offset", glm::value_ptr(m_AutomaticSizingOffset));

						ImGui::InputFloat2("Fixed Size", glm::value_ptr(m_FixedSize));

						ImGui::TreePop();
					}

					if (ImGui::TreeNodeEx("Automatic Positioning", ImGuiTreeNodeFlags_DefaultOpen))
					{

						ImGui::Checkbox("Automatic Positioning", &m_AutomaticPositioning);

						ImGui::InputFloat3("Automatic Position Offset", glm::value_ptr(m_AutomaticPositionOffset));

						if (ImGui::TreeNodeEx("Anchoring", ImGuiTreeNodeFlags_DefaultOpen))
						{
							//horiz & vert alignment
							ImGui::Text("Do this in the code i can't be bothered rn");
							

							ImGui::TreePop();
						}

						

						ImGui::TreePop();
					}


					ImGui::TreePop();
				}
			}

			void AutoRect()
			{
				if (UIRect* parentRect = dynamic_cast<UIRect*>(m_Parent))
				{
					if (m_AutomaticSizing)
					{
						m_Size = parentRect->m_Size * m_AutomaticSizingPercent;
						m_Size += m_AutomaticSizingOffset;
						if (m_FixedSize.x != -1) m_Size.x = m_FixedSize.x;
						if (m_FixedSize.y != -1) m_Size.y = m_FixedSize.y;
					}

					if (m_AutomaticPositioning)
					{
						//reset position
						m_Position = glm::vec3(0, 0, m_Position.z);
						//horizontal
						if (m_HorizontalAlignment == Left)
						{
							m_Position.x -= (parentRect->m_Size.x / 2.0f) - (m_Size.x / 2);
						}
						else if (m_HorizontalAlignment == Right)
						{
							m_Position.x += (parentRect->m_Size.x / 2.0f) - (m_Size.x / 2);
						}

						//vertical
						if (m_VerticalAlignment == Up)
						{
							m_Position.y += (parentRect->m_Size.y / 2.0f) - (m_Size.y / 2);
						}
						else if (m_VerticalAlignment == Down)
						{
							m_Position.y -= (parentRect->m_Size.y / 2.0f) - (m_Size.y / 2);
						}

						m_Position += m_AutomaticPositionOffset;
						
						UpdateLocalTransform();
						
					}
				}
				
			}

			virtual void PropagateUpdate() override
			{
				AutoRect();
				UINode::PropagateUpdate();
			}

			//virtual void OnUpdate(Timestep ts) override;

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					if (m_TextureResource != nullptr)
					{	
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, GetUUID(), 1, m_Color);
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

		REGISTER_SERIALIZABLE_NODE(UIRect);

	}
}