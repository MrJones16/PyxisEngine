#include "Node.h"
#include "Node.h"
#include "Node.h"
#include "pxpch.h"
#include "Pyxis/Renderer/Renderer2D.h"
#include "imgui.h"
#include "imgui_stdlib.h"




namespace Pyxis
{
	Ref<Node> Node::DeserializeNode(json& j)
	{
		if (j.contains("Type"))
		{
			UUID ID = j.contains("UUID") ? j["UUID"].get<UUID>() : 0;
			Ref<Node> newNode = NodeRegistry::getInstance().createInstance(j["Type"], ID);
			if (newNode != nullptr)
			{
				newNode->Deserialize(j);				
			}
			return newNode;
		}
		return nullptr;
	}
	Node::Node(const std::string& name)
		: m_UUID(GenerateUUID()), m_Name(name)
	{
		Node::Nodes[m_UUID] = this;
	}

	Node::Node(UUID id)
		: m_UUID(UseExistingUUID(id))
	{
		Node::Nodes[m_UUID] = this;
	}

	Node::~Node()
	{				
		Node::Nodes[m_UUID] = nullptr;
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

	void Node::Serialize(json& j)
	{
		j["Type"] = "Node";
		j["UUID"] = m_UUID;
		j["m_Name"] = m_Name;
		//j["m_Parent"] = m_Parent ? m_Parent->m_UUID : (UUID)0;
		j["m_Enabled"] = m_Enabled;

		for (auto child : m_Children)
		{
			json cj;
			child->Serialize(cj);
			j["m_Children"] += cj;
		}
	}

	void Node::Deserialize(json& j)
	{
		if (j.contains("m_Name")) j.at("m_Name").get_to(m_Name);
		if (j.contains("m_Enabled")) j.at("m_Enabled").get_to(m_Enabled);
		for (auto jc : j["m_Children"])
		{
			if (j.contains("Type"))
			{
				std::string type = jc["Type"];
				UUID ID = jc.contains("UUID") ? jc["UUID"].get<UUID>() : 0;
				Ref<Node> newNode = NodeRegistry::getInstance().createInstance(type, ID);
				if (newNode != nullptr)
				{
					m_Children.push_back(newNode);
					newNode->m_Parent = this;
					newNode->Deserialize(jc);
				}
				
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
	void Node::OnInspectorRender()
	{
		std::string ID = "ID: " + std::to_string(m_UUID);
		ImGui::Text(ID.c_str());
		ImGui::InputText("##Name", &m_Name);
		ImGui::Checkbox("Enabled", &m_Enabled);		
	}

	

}
