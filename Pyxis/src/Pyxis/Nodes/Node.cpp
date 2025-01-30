#include "pxpch.h"
#include "Node.h"
#include "Pyxis/Renderer/Renderer2D.h"
#include "imgui.h"
#include "imgui_stdlib.h"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"


namespace Pyxis
{

	Node::Node(const std::string& name)
		: m_ID(++NodeCounter), m_Name(name)
	{
		Node::Nodes[m_ID] = this;
	}

	Node::~Node()
	{
		Nodes[m_ID] = nullptr;//won't actuall remove from the collection since this might happen during iteration
		for (auto child : m_Children)
		{
			child->m_Parent = nullptr;
		}
	}

	/*void Node::OnUpdate(Timestep ts)
	{
		if (m_Enabled)
		for (Ref<Node> node : m_Children)
		{
			node->OnUpdate(ts);
		}
	}*/

	/*void Node::OnRender()
	{
		if (m_Enabled)
		for (Ref<Node> node : m_Children)
		{
			node->OnRender();
		}
	}*/

	void Node::AddChild(const Ref<Node>& child)
	{
		m_Children.push_back(child);
		if (child->m_Parent != nullptr)
		{
			//child was already a child of another object, so un-parent it
			child->m_Parent->RemoveChild(child);
		}
		child->m_Parent = this;
	}

	void Node::RemoveChild(const Ref<Node>& child) 
	{
		for (auto it = m_Children.begin(); it != m_Children.end(); it++)
		{
			if (it->get() == child.get())
			{
				m_Children.erase(it);
				child->m_Parent = nullptr;
				break;
			}
		}
	}

	void Node::OnUpdate(Timestep ts)
	{

	}

	void Node::OnFixedUpdate()
	{

	}

	void Node::OnRender()
	{

	}

	/// <summary>
	/// The base Inspector Render which allows editing of Translation, Rotation, and Scale.
	/// </summary>
	void Node::InspectorRender()
	{
		std::string ID = "ID: " + std::to_string(m_ID);
		ImGui::Text(ID.c_str());
		ImGui::InputText("##Name", &m_Name);
		ImGui::Checkbox("Enabled", &m_Enabled);
		if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::DragFloat3("Position", glm::value_ptr(m_Position)))
			{
				UpdateLocalTransform();
			}
			if (ImGui::DragFloat3("Rotation", glm::value_ptr(m_Rotation)))
			{
				UpdateLocalTransform();
			}
			if (ImGui::DragFloat3("Scale", glm::value_ptr(m_Scale)))
			{
				UpdateLocalTransform();
			}
			ImGui::TreePop();
		}
	}

	void Node::UpdateLocalTransform()
	{
		//PX_TRACE("Updated Transform");
		m_LocalTransform = glm::translate(glm::mat4(1), m_Position);
		m_LocalTransform = glm::rotate(m_LocalTransform, glm::radians(m_Rotation.x), {-1, 0, 0});
		m_LocalTransform = glm::rotate(m_LocalTransform, glm::radians(m_Rotation.y), { 0,-1, 0 });
		m_LocalTransform = glm::rotate(m_LocalTransform, glm::radians(m_Rotation.z), { 0, 0,-1 });
		m_LocalTransform = glm::scale(m_LocalTransform, m_Scale);
	}

	glm::mat4 Node::GetWorldTransform()
	{
		if (m_Parent != nullptr)
		{
			//TODO: Test if this is the correct order
			return m_Parent->GetWorldTransform() * m_LocalTransform;
		}
		else
		{
			return m_LocalTransform;
		}

	}

	void Node::ResetLocalTransform()
	{
		m_Position = glm::vec3(0);
		m_Scale = glm::vec3(1);
		m_Rotation = glm::vec3(0);
		m_LocalTransform = glm::mat4(1);
	}

	void Node::SetLocalTransform(const glm::mat4& transform)
	{
		glm::quat rotation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(transform, m_Scale, rotation, m_Position, skew, perspective);

		m_Rotation = -glm::degrees(glm::eulerAngles(rotation));

		UpdateLocalTransform();
	}


	glm::mat4& Node::GetLocalTransform()
	{
		return m_LocalTransform;
	}

	void Node::Translate(glm::vec3 translation)
	{
		m_Position += translation;
		UpdateLocalTransform();
	}

	void Node::Rotate(glm::vec3 rotation)
	{
		m_Rotation += rotation;
		UpdateLocalTransform();
	}

	void Node::SetScale(glm::vec3 scale)
	{
		m_Scale = scale;
		UpdateLocalTransform();
	}

	void Node::Scale(glm::vec3 scale)
	{
		m_Scale *= scale;
		UpdateLocalTransform();
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
