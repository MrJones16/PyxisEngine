#include "pxpch.h"
#include "Node.h"
#include "Pyxis/Renderer/Renderer2D.h"
#include "imgui.h"


namespace Pyxis
{

	Node::Node(const std::string& name)
		: m_ID(++NodeCounter)
	{

	}

	void Node::OnUpdate(Timestep ts)
	{
		for (Ref<Node> node : m_Children)
		{
			node->OnUpdate(ts);
		}
	}

	void Node::OnRender()
	{
		for (Ref<Node> node : m_Children)
		{
			node->OnRender();
		}
	}

	void Node::AddChild(const Ref<Node>& child)
	{
		m_Children.push_back(child);
	}

	void Node::RemoveChild(const Ref<Node>& child) 
	{
		for (auto it = m_Children.begin(); it != m_Children.end(); it++)
		{
			if (it->get() == child.get())
			{
				m_Children.erase(it);
				break;
			}
		}
	}

	void Node::InspectorRender()
	{
		bool opened = ImGui::TreeNodeEx("Transform");
		if (opened)
		{
			
			
			glm::vec3 Position = m_LocalTransform[3];
			glm::vec3 posCopy = glm::vec3(Position);
			ImGui::InputFloat3("Position", glm::value_ptr(Position));
			glm::vec3 diff = Position - posCopy;
			m_LocalTransform = glm::translate(m_LocalTransform, diff);
			
			ImGui::TreePop();
		}
		//ImGui::End();
	}

	glm::mat4 Node::GetTransform()
	{
		if (m_Parent != nullptr)
		{
			//TODO: Test if this is the correct order
			return m_LocalTransform * m_Parent->GetTransform();
		}
		else
		{
			return m_LocalTransform;
		}
	}


	//NodeWithSprite::NodeWithSprite(const std::string& name) : Node(name)
	//{
	//	//Node::Node(name);
	//	m_Name = name;
	//	m_Transform = glm::mat4(1);
	//	m_Texture = nullptr;
	//}
	//NodeWithSprite::NodeWithSprite(Ref<Texture2D> texture, const std::string& name)
	//{
	//	m_Transform = glm::mat4(1);
	//	m_Texture = texture;
	//}
	//void NodeWithSprite::OnUpdate(Timestep ts)
	//{
	//	Node::OnUpdate(ts);
	//	if (m_Texture)
	//	{
	//		Renderer2D::DrawQuad(m_Transform, m_Texture);
	//	}
	//	else
	//	{
	//		Renderer2D::DrawQuad(m_Transform, { 1,1,1,1 });
	//	}
	//}

}
