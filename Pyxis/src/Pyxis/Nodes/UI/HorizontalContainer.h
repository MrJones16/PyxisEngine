#pragma once
#include "UIRect.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that will update the positions of it's children to be in an organized fashion
		/// </summary>
		class HorizontalContainer : public UIRect
		{
		public:

			//overriding default color & size so easier to debug

			float m_Padding = 0.0f;
			float m_Gap = 0.2f;
			
			bool m_ShowRegion = false;
			Direction m_Arrangement = Center;

			HorizontalContainer(const std::string& name = "HorizontalContainer") : UIRect(name)
			{
				
			}

			HorizontalContainer(UUID id) : UIRect(id)
			{

			}

			virtual ~HorizontalContainer() = default;

			// Serialization
			virtual void Serialize(json& j) override
			{
				UIRect::Serialize(j);
				j["Type"] = "HorizontalContainer";

				j["m_Padding"] = m_Padding;
				j["m_Gap"] = m_Gap;
				j["m_ShowRegion"] = m_ShowRegion;
				j["m_Arrangement"] = (int)m_Arrangement;
			}
			virtual void Deserialize(json& j) override
			{
				UIRect::Deserialize(j);

				if (j.contains("m_Padding")) j.at("m_Padding").get_to(m_Padding);
				if (j.contains("m_Gap")) j.at("m_Gap").get_to(m_Gap);
				if (j.contains("m_ShowRegion")) j.at("m_ShowRegion").get_to(m_ShowRegion);
				if (j.contains("m_Arrangement")) j.at("m_Arrangement").get_to(m_Arrangement);
			}


			virtual void AddChild(const Ref<Node>& child) override
			{
				UIRect::AddChild(child);
				RearrangeChildren();
			}

			virtual void RemoveChild(const Ref<Node>& child) override
			{
				UIRect::RemoveChild(child);
				RearrangeChildren();
			}			

			virtual void OnInspectorRender() override
			{
				UIRect::OnInspectorRender();
				if (ImGui::TreeNodeEx("HorizontalContainer", ImGuiTreeNodeFlags_DefaultOpen))
				{
					if (ImGui::DragFloat("Gap", &m_Gap)) RearrangeChildren();

					if (ImGui::Checkbox("Show Region", &m_ShowRegion));

					if (ImGui::Button("Rearrange Children")) RearrangeChildren();

					if (ImGui::TreeNodeEx("m_Arrangement", ImGuiTreeNodeFlags_DefaultOpen))
					{
						if (ImGui::Button("Right"))		{ m_Arrangement = Direction::Right; RearrangeChildren();}
						if (ImGui::Button("Left"))		{ m_Arrangement = Direction::Left; RearrangeChildren(); }
						if (ImGui::Button("Center"))	{ m_Arrangement = Direction::Center; RearrangeChildren();}
						ImGui::TreePop();
					}

					
					ImGui::TreePop();
				}
			}

			virtual void PropagateUpdate() override
			{
				AutoRect();
				UINode::PropagateUpdate();
				RearrangeChildren();
			}

			virtual void OnRender() override
			{
				if (m_Enabled && m_ShowRegion)
				{
					if (m_TextureResource != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_TextureResource->m_Texture, GetUUID());
					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, GetUUID());
					}

				}

			}

			void SetHeightFromChildren()
			{
				float maxHeight = 0;
				for (auto& child : m_Children)
				{
					if (UIRect* rect = dynamic_cast<UIRect*>(child))
					{
						if (rect->m_Size.y > maxHeight) maxHeight = rect->m_Size.y;
					}

				}
				m_Size.y = maxHeight;
			}
			
			void SetSizeFromChildren()
			{
				float maxHeight = 0;
				float sumWidth = -m_Gap + (m_Padding * 2.0f);
				for (auto& child : m_Children)
				{
					if (UIRect* rect = dynamic_cast<UIRect*>(child))
					{
						if (rect->m_Size.y > maxHeight) maxHeight = rect->m_Size.y;

						sumWidth += m_Gap + rect->m_Size.x;
					}

				}
				m_Size = { sumWidth, maxHeight };
			}

			void RearrangeChildren()
			{
				//re-position the children!

				//first, lets get our current position
				glm::vec3 startPosition = glm::vec3(0, 0, 0);

				float maxShiftX = (m_Size.x / 2) - m_Padding;
				float sumHorizontal = -m_Gap;

				//we need to set which corner to start in based on horizontal and vertical alignment
				if (m_Arrangement == Left)
				{
					//shift left half of the size to reach left wall, and give padding space
					startPosition += glm::vec3(-maxShiftX, 0, 0);
				}
				else if (m_Arrangement == Right)
				{
					startPosition += glm::vec3(maxShiftX, 0, 0);
				}
				else
				{
					//since we are centered, we have to find the total length of the items,
					//with a max of the border of the HorizontalContainer
					
					for (auto& child : m_Children)
					{
						if (UIRect* rect = dynamic_cast<UIRect*>(child))
						{					
							sumHorizontal += rect->m_Size.x + m_Gap;
						}
					}
					
					startPosition += glm::vec3(-sumHorizontal / 2, 0, 0);					
					
				}

				glm::vec3 position = startPosition;
				for (auto& child : m_Children)
				{
					if (UIRect* rect = dynamic_cast<UIRect*>(child))
					{										

						//since we have adjusted the start position, lets reset the transform, and set it to the position
						//for now, and then we will move it based on its size
						rect->ResetLocalTransform();
						rect->Translate(position);

						//for both vertical & horizontal alignment, decide if we need to shift a direction
						//to offset the size of the object
						switch (m_Arrangement)
						{
						case Pyxis::UI::Direction::Left:
							rect->Translate({ (rect->m_Size.x / 2), 0, 0 });
							break;
						case Pyxis::UI::Direction::Center:
							rect->Translate({ (rect->m_Size.x / 2), 0, 0 });
							break;
						case Pyxis::UI::Direction::Right:
							rect->Translate({ -(rect->m_Size.x / 2), 0, 0 });
							break;
						default:
							break;
						}


						//finally,
						//shift position the direction after setting an item
						switch (m_Arrangement)
						{
						case Pyxis::UI::Direction::Center:
						case Pyxis::UI::Direction::Left:
							position.x += m_Gap + (rect->m_Size.x);
							break;
						case Pyxis::UI::Direction::Right:
							position.x -= m_Gap + (rect->m_Size.x);
							break;
						default:
							break;
						}

					}
				}
			}
		};
		REGISTER_SERIALIZABLE_NODE(HorizontalContainer);

	}
}