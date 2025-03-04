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

	}

	Node::Node(UUID id)
		: m_UUID(UseExistingUUID(id))
	{

	}

	Node::~Node()
	{				

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
		if (child == nullptr) return;

		m_Children.push_back(child.get());
		if (child->m_Parent != nullptr)
		{
			//child was already a child of another object, so un-parent it
			child->m_Parent->RemoveChild(child);
		}
		child->m_Parent = this;
	}

	void Node::RemoveChild(const Ref<Node>& child) 
	{
		if (child == nullptr) return;

		// iterates through children, removing the child
		// when found
		for (auto it = m_Children.begin(); it != m_Children.end(); it++)
		{
			if (*it == child.get())
			{
				m_Children.erase(it);
				child->m_Parent = nullptr;
				return;				
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
					Node::Nodes[newNode->GetUUID()] = newNode;
					m_Children.push_back(newNode.get());
					newNode->m_Parent = this;
					newNode->Deserialize(jc);
				}
				
			}			
		}
	}

	std::vector<uint8_t> Node::SerializeBinary()
	{
		json j;
		Serialize(j);
		//msgpack seems to be the most compact
		//return json::to_bson(j);
		return json::to_msgpack(j);
	}

	void Node::DeserializeBinary(std::vector<uint8_t> msgpack)
	{
		json j = json::from_msgpack(msgpack);
		Deserialize(j);
	}

	

	void Node::QueueFree()
	{
		Node::NodesToDestroyQueue.push(m_UUID);
	}

	void Node::QueueFreeHierarchy()
	{
		for (auto child : m_Children)
		{
			child->QueueFreeHierarchy();
		}
		Node::NodesToDestroyQueue.push(m_UUID);
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
