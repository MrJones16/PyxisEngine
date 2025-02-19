#pragma once
#include "UIRect.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that will update the positions of it's children to be in an organized fashion
		/// </summary>
		class Container : public UIRect
		{
		public:

			//overriding default color & size so easier to debug

			float m_Padding = 0.0f;
			float m_Gap = 0.2f;
			
			bool m_ShowRegion = false;

			Direction m_HorizontalAlignment = Left;
			Direction m_VerticalAlignment = Up;
			Direction m_Direction = Right;

			Container(const std::string& name = "Container") : UIRect(name)
			{
				
			}

			Container(UUID id) : UIRect(id)
			{

			}

			virtual ~Container() = default;

			//Serialization
			virtual void Serialize(json& j) override
			{
				UIRect::Serialize(j);
				j["Type"] = "Container";


				j["m_Padding"] = m_Padding;
				j["m_Gap"] = m_Gap;
				j["m_ShowRegion"] = m_ShowRegion;
				j["m_HorizontalAlignment"] = m_HorizontalAlignment;
				j["m_VerticalAlignment"] = m_VerticalAlignment;
				j["m_Direction"] = m_Direction;
			}
			
			virtual void Deserialize(json& j) override
			{
				UIRect::Deserialize(j);

				m_Padding = j["m_Padding"];
				m_Gap = j["m_Gap"];
				m_ShowRegion = j["m_ShowRegion"];
				m_HorizontalAlignment = j["m_HorizontalAlignment"];
				m_VerticalAlignment = j["m_VerticalAlignment"];
				m_Direction = j["m_Direction"];
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
				if (ImGui::TreeNodeEx("Container", ImGuiTreeNodeFlags_DefaultOpen))
				{
					//Size
					if (ImGui::DragFloat2("Size", glm::value_ptr(m_Size))) RearrangeChildren();

					if (ImGui::DragFloat("Gap", &m_Gap)) RearrangeChildren();

					if (ImGui::Checkbox("Show Region", &m_ShowRegion));

					if (ImGui::Button("Rearrange Children")) RearrangeChildren();

					if (ImGui::TreeNodeEx("Direction", ImGuiTreeNodeFlags_DefaultOpen))
					{

						if (ImGui::Button("Right"))		{ m_Direction =	Direction::Right; RearrangeChildren();}
						if (ImGui::Button("Left"))		{ m_Direction = Direction::Left; RearrangeChildren();}
						if (ImGui::Button("Up"))		{ m_Direction = Direction::Up; RearrangeChildren();}
						if (ImGui::Button("Down"))		{ m_Direction = Direction::Down; RearrangeChildren();}
						ImGui::TreePop();
					}

					if (ImGui::TreeNodeEx("Horizontal Alignment", ImGuiTreeNodeFlags_DefaultOpen))
					{
						if (ImGui::Button("Left"))	{ m_HorizontalAlignment = Direction::Left; RearrangeChildren();}
						if (ImGui::Button("Right"))	{ m_HorizontalAlignment = Direction::Right; RearrangeChildren();}
						if (ImGui::Button("Center")){ m_HorizontalAlignment = Direction::Center; RearrangeChildren();}
						ImGui::TreePop();
					}
					if (ImGui::TreeNodeEx("Vertical Alignment", ImGuiTreeNodeFlags_DefaultOpen))
					{
						if (ImGui::Button("Up"))	{ m_VerticalAlignment = Direction::Up; RearrangeChildren();}
						if (ImGui::Button("Down"))	{ m_VerticalAlignment = Direction::Down; RearrangeChildren();}
						if (ImGui::Button("Center")) { m_VerticalAlignment = Direction::Center; RearrangeChildren();}
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

			void RearrangeChildren()
			{
				//re-position the children!

				//first, lets get our current position
				glm::vec3 startPosition = glm::vec3(0, 0, 0);

				float maxShiftX = (m_Size.x / 2) - m_Padding;
				float maxShiftY = (m_Size.y / 2) - m_Padding;

				float maxHorizontal = m_Size.x - (m_Padding * 2), maxVertical = m_Size.y - (m_Padding * 2);
				float sumVertical = -m_Gap, sumHorizontal = -m_Gap;

				//we need to set which corner to start in based on horizontal and vertical alignment
				if (m_HorizontalAlignment == Left)
				{
					//shift left half of the size to reach left wall, and give padding space
					startPosition += glm::vec3(-maxShiftX, 0, 0);
				}
				else if (m_HorizontalAlignment == Right)
				{
					startPosition += glm::vec3(maxShiftX, 0, 0);
				}
				else
				{
					//since we are centered, we have to find the total length of the items,
					//with a max of the border of the container
					if (m_Direction == Left || m_Direction == Right)
					{
						for (auto& child : m_Children)
						{
							
							if (UIRect* rect = dynamic_cast<UIRect*>(child))
							{
								//keep adding a shift until a child is too big to fit
								if (sumHorizontal + rect->m_Size.x + m_Gap > maxHorizontal)
									break;

								sumHorizontal += rect->m_Size.x + m_Gap;
							}
						}
						if (m_Direction == Left)
							startPosition += glm::vec3(-sumHorizontal, 0, 0);
						else
							startPosition += glm::vec3(sumHorizontal, 0, 0);
					}

				}

				if (m_VerticalAlignment == Up)
				{
					startPosition += glm::vec3(0, maxShiftY, 0);
				}
				else if (m_VerticalAlignment == Down)
				{
					startPosition += glm::vec3(0, -maxShiftY, 0);
				}
				else
				{
					//since we are centered, we have to find the total length of the items,
					//with a max of the border of the container
					if (m_Direction == Up || m_Direction == Down)
					{
						for (auto& child : m_Children)
						{
							
							if (UIRect* rect = dynamic_cast<UIRect*>(child))
							{
								//keep adding a shift until a child is too big to fit
								if (sumVertical + rect->m_Size.y + m_Gap > maxVertical)
									break;

								sumVertical += rect->m_Size.y + m_Gap;
							}
						}
						if (m_Direction == Up)
							startPosition += glm::vec3(0, -sumVertical, 0);
						else
							startPosition += glm::vec3(0, sumVertical, 0);
					}
				}


				//actually move the positions now that we know the starting position

				glm::vec3 position = startPosition;
				//glm::mat4 transform = glm::translate(glm::mat4(1), startPosition);

				float nextLineShift = 0;
				for (auto& child : m_Children)
				{

					
					if (UIRect* rect = dynamic_cast<UIRect*>(child))
					{

						//first lets see if we are past the limit
						switch (m_Direction)
						{
						case Pyxis::UI::Direction::Up:
							if (position.y + (rect->m_Size.y) > maxShiftY)
							{
								//with this next object, we will be past the vertical limit, so lets shift against alignment
								if (m_HorizontalAlignment == Left)
									startPosition.x -= nextLineShift;
								else
									startPosition.x += nextLineShift;
								position = startPosition;
								nextLineShift = m_Gap;
							}
							break;
						case Pyxis::UI::Direction::Down:
							if (position.y - (rect->m_Size.y) < -maxShiftY)
							{
								//with this next object, we will be past the vertical limit, so lets shift against alignment
								if (m_HorizontalAlignment == Left)
									startPosition.x -= nextLineShift;
								else
									startPosition.x += nextLineShift;
								position = startPosition;
								nextLineShift = m_Gap;
							}
							break;
						case Pyxis::UI::Direction::Left:
							if (position.x - (rect->m_Size.x) < -maxShiftX)
							{
								//with this next object, we will be past the vertical limit, so lets shift against alignment
								if (m_VerticalAlignment == Up)
									startPosition.y -= nextLineShift;
								else
									startPosition.y += nextLineShift;
								position = startPosition;
								nextLineShift = m_Gap;
							}
							break;
						case Pyxis::UI::Direction::Right:
							if (position.x + (rect->m_Size.x) > maxShiftX)
							{
								//with this next object, we will be past the vertical limit, so lets shift against alignment
								if (m_VerticalAlignment == Up)
									startPosition.y -= nextLineShift;
								else
									startPosition.y += nextLineShift;
								position = startPosition;
								nextLineShift = m_Gap;
							}
							break;
						case Pyxis::UI::Direction::None:
						case Pyxis::UI::Direction::Center:
						default:
							break;
						}

						switch (m_Direction)
						{
						case Pyxis::UI::Direction::Up:
						case Pyxis::UI::Direction::Down:
						{
							if (rect->m_Size.x + m_Gap > nextLineShift) nextLineShift = rect->m_Size.x + m_Gap;
						}
						case Pyxis::UI::Direction::Left:
						case Pyxis::UI::Direction::Right:
						{
							if (rect->m_Size.y + m_Gap > nextLineShift) nextLineShift = rect->m_Size.y + m_Gap;
						}
						default:
							break;
						}

						//since we have adjusted the start position, lets reset the transform, and set it to the position
						//for now, and then we will move it based on its size
						rect->ResetLocalTransform();
						rect->Translate(position);

						//for both vertical & horizontal alignment, decide if we need to shift a direction
						//to offset the size of the object
						switch (m_HorizontalAlignment)
						{

						case Pyxis::UI::Direction::Left:
							rect->Translate({ (rect->m_Size.x / 2), 0, 0 });
							break;
						case Pyxis::UI::Direction::Right:
							rect->Translate({ -(rect->m_Size.x / 2), 0, 0 });
							break;
						case Pyxis::UI::Direction::Center:
						case Pyxis::UI::Direction::Up:
						case Pyxis::UI::Direction::Down:
						case Pyxis::UI::Direction::None:
						default:
							break;
						}
						switch (m_VerticalAlignment)
						{

						case Pyxis::UI::Direction::Up:
							rect->Translate({ 0, -(rect->m_Size.y / 2), 0 });
							break;
						case Pyxis::UI::Direction::Down:
							rect->Translate({ 0, (rect->m_Size.y / 2), 0 });
							break;
						case Pyxis::UI::Direction::Center:
						case Pyxis::UI::Direction::Left:
						case Pyxis::UI::Direction::Right:
						case Pyxis::UI::Direction::None:
						default:
							break;
						}

						//finally,
						//shift position the direction after setting an item
						switch (m_Direction)
						{
						case Pyxis::UI::Direction::Up:
							position.y += m_Gap + (rect->m_Size.y);
							break;
						case Pyxis::UI::Direction::Down:
							position.y -= m_Gap + (rect->m_Size.y);
							break;
						case Pyxis::UI::Direction::Left:
							position.x -= m_Gap + (rect->m_Size.x);
							break;
						case Pyxis::UI::Direction::Right:
							position.x += m_Gap + (rect->m_Size.x);
							break;
						case Pyxis::UI::Direction::None:
						case Pyxis::UI::Direction::Center:
						default:
							break;
						}

					}
				}
			}
		};
		REGISTER_SERIALIZABLE_NODE(Container);

	}
}