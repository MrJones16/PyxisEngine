#pragma once
#include "UIRect.h"

namespace Pyxis
{
	namespace UI
	{
		/// <summary>
		/// A UI Node that will update the positions of it's children to be in an organized fashion
		/// </summary>
		class UIContainer : public UIRect
		{
		public:

			//overriding default color & size so easier to debug

			float m_Padding = 0.0f;
			float m_Gap = 0.2f;
			
			enum ContainerEnum : int
			{
				None,

				Up, Down, Left, Right, Center
			};

			ContainerEnum m_HorizontalAlignment = Left;
			ContainerEnum m_VerticalAlignment = Up;
			ContainerEnum m_Direction = Right;

			UIContainer(const std::string& name = "UIContainer") : UIRect(name)
			{

			}

			UIContainer(Ref<Texture2D> texture, const std::string& name = "UIContainer") : UIRect(texture, name)
			{

			}

			UIContainer(const glm::vec4& color, const std::string& name = "UIContainer") : UIRect(color, name)
			{

			}

			virtual ~UIContainer() = default;

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

			virtual void InspectorRender() override
			{
				UIRect::InspectorRender();
				if (ImGui::TreeNodeEx("Container", ImGuiTreeNodeFlags_DefaultOpen))
				{
					//Size
					if (ImGui::DragFloat2("Size", glm::value_ptr(m_Size))) RearrangeChildren();

					if (ImGui::DragFloat("Gap", &m_Gap)) RearrangeChildren();

					if (ImGui::TreeNodeEx("Direction", ImGuiTreeNodeFlags_DefaultOpen))
					{

						if (ImGui::Button("Right"))		{ m_Direction =	ContainerEnum::Right; RearrangeChildren();}
						if (ImGui::Button("Left"))		{ m_Direction = ContainerEnum::Left; RearrangeChildren();}
						if (ImGui::Button("Up"))		{ m_Direction = ContainerEnum::Up; RearrangeChildren();}
						if (ImGui::Button("Down"))		{ m_Direction = ContainerEnum::Down; RearrangeChildren();}
						ImGui::TreePop();
					}

					if (ImGui::TreeNodeEx("Horizontal Alignment", ImGuiTreeNodeFlags_DefaultOpen))
					{
						if (ImGui::Button("Left"))	{ m_HorizontalAlignment = ContainerEnum::Left; RearrangeChildren();}
						if (ImGui::Button("Right"))	{ m_HorizontalAlignment = ContainerEnum::Right; RearrangeChildren();}
						if (ImGui::Button("Center")){ m_HorizontalAlignment = ContainerEnum::Center; RearrangeChildren();}
						ImGui::TreePop();
					}
					if (ImGui::TreeNodeEx("Vertical Alignment", ImGuiTreeNodeFlags_DefaultOpen))
					{
						if (ImGui::Button("Up"))	{ m_VerticalAlignment = ContainerEnum::Up; RearrangeChildren();}
						if (ImGui::Button("Down"))	{ m_VerticalAlignment = ContainerEnum::Down; RearrangeChildren();}
						if (ImGui::Button("Center")) { m_VerticalAlignment = ContainerEnum::Center; RearrangeChildren();}
						ImGui::TreePop();
					}
					ImGui::TreePop();
				}
			}

			virtual void OnRender() override
			{
				if (m_Enabled)
				{
					if (m_Texture != nullptr)
					{
						//we have a texture, so display it!
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });

						//TODO: Test ordering
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Texture, GetID());
					}
					else
					{
						//just draw the color as the square
						glm::mat4 sizeMat = glm::scale(glm::mat4(1.0f), { m_Size.x, m_Size.y, 1 });
						Renderer2D::DrawQuadEntity(GetWorldTransform() * sizeMat, m_Color, GetID());
					}

				}

			}

			void RearrangeChildren()
			{
				//re-position the children!

				//first, lets get our current position
				glm::vec3 startPosition = glm::vec3(0, 0, 1);

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
							if (UIRect* rect = dynamic_cast<UIRect*>(child.get()))
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
							if (UIRect* rect = dynamic_cast<UIRect*>(child.get()))
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

					if (UIRect* rect = dynamic_cast<UIRect*>(child.get()))
					{

						//first lets see if we are past the limit
						switch (m_Direction)
						{
						case Pyxis::UI::UIContainer::Up:
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
						case Pyxis::UI::UIContainer::Down:
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
						case Pyxis::UI::UIContainer::Left:
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
						case Pyxis::UI::UIContainer::Right:
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
						case Pyxis::UI::UIContainer::None:
						case Pyxis::UI::UIContainer::Center:
						default:
							break;
						}

						switch (m_Direction)
						{
						case Pyxis::UI::UIContainer::Up:
						case Pyxis::UI::UIContainer::Down:
						{
							if (rect->m_Size.x + m_Gap > nextLineShift) nextLineShift = rect->m_Size.x + m_Gap;
						}
						case Pyxis::UI::UIContainer::Left:
						case Pyxis::UI::UIContainer::Right:
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

						case Pyxis::UI::UIContainer::Left:
							rect->Translate({ (rect->m_Size.x / 2), 0, 0 });
							break;
						case Pyxis::UI::UIContainer::Right:
							rect->Translate({ -(rect->m_Size.x / 2), 0, 0 });
							break;
						case Pyxis::UI::UIContainer::Center:
						case Pyxis::UI::UIContainer::Up:
						case Pyxis::UI::UIContainer::Down:
						case Pyxis::UI::UIContainer::None:
						default:
							break;
						}
						switch (m_VerticalAlignment)
						{

						case Pyxis::UI::UIContainer::Up:
							rect->Translate({ 0, -(rect->m_Size.y / 2), 0 });
							break;
						case Pyxis::UI::UIContainer::Down:
							rect->Translate({ 0, (rect->m_Size.y / 2), 0 });
							break;
						case Pyxis::UI::UIContainer::Center:
						case Pyxis::UI::UIContainer::Left:
						case Pyxis::UI::UIContainer::Right:
						case Pyxis::UI::UIContainer::None:
						default:
							break;
						}

						//finally,
						//shift position the direction after setting an item
						switch (m_Direction)
						{
						case Pyxis::UI::UIContainer::Up:
							position.y += m_Gap + (rect->m_Size.y);
							break;
						case Pyxis::UI::UIContainer::Down:
							position.y -= m_Gap + (rect->m_Size.y);
							break;
						case Pyxis::UI::UIContainer::Left:
							position.x -= m_Gap + (rect->m_Size.x);
							break;
						case Pyxis::UI::UIContainer::Right:
							position.x += m_Gap + (rect->m_Size.x);
							break;
						case Pyxis::UI::UIContainer::None:
						case Pyxis::UI::UIContainer::Center:
						default:
							break;
						}

					}
				}
			}
		};

	}
}