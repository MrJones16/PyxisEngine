#pragma once

#include <glm/glm.hpp>
#include "Pyxis/Renderer/Texture.h"
#include "Pyxis/Core/Timestep.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

namespace Pyxis
{

	//Node is the base entity class that lives in a scene heirarchy.
	//Nodes is a collection of all living nodes
	// s_HoveredNodeID is the ID of the node with a mouse hovering over it, set by the scene layer.
	// It belongs to Node{} because it helps to be accessible from nodes, to know if they are hovered / wanting to be 
	// interacted with. 
	//
	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		inline static uint32_t NodeCounter = 1;

		inline static std::unordered_map<uint32_t, Node*> Nodes; 
		inline static uint32_t s_HoveredNodeID = 0;
		
		Node(const std::string& name = "Node");
		virtual ~Node();

		/// <summary>
		/// Tries to remove this object from its parent. However, if this is still referenced by something, it won't be deleted.
		/// If you hold a shared pointer to a node, it will keep it alive! use weak if needed
		/// </summary>
		virtual void QueueFree() { if (m_Parent != nullptr) m_Parent->RemoveChild(shared_from_this()); }

		virtual void OnUpdate(Timestep ts);
		virtual void OnFixedUpdate();
		virtual void OnRender();
		virtual void OnImGuiRender() {};
		virtual void InspectorRender();

		uint32_t GetID() { return m_ID; }

		virtual void AddChild(const Ref<Node>& child);
		virtual void RemoveChild(const Ref<Node>& child);
		
		/// <summary>
		/// Gets this transform 
		/// </summary>
		/// <returns></returns>
		virtual glm::mat4 GetWorldTransform();

		virtual void ResetLocalTransform();
		virtual void SetLocalTransform(const glm::mat4& transform);
		glm::mat4& GetLocalTransform();
		virtual void Translate(glm::vec3 translation);
		virtual void Rotate(glm::vec3 rotation);
		virtual void SetScale(glm::vec3 scale);
		virtual void Scale(glm::vec3 scale);

		std::string m_Name = "Node";
		Node* m_Parent = nullptr;
		std::vector<Ref<Node>> m_Children;

		bool m_Enabled = true;

	protected:

		/// <summary>
		/// Uses the private local position rotation and scale to set the local transform
		/// </summary>
		virtual void UpdateLocalTransform();
		glm::mat4 m_LocalTransform = glm::mat4(1);
		glm::vec3 m_Position = glm::vec3(0);
		glm::vec3 m_Rotation = glm::vec3(0);
		glm::vec3 m_Scale = glm::vec3(1);

		const uint32_t m_ID = ++NodeCounter;
	};

}