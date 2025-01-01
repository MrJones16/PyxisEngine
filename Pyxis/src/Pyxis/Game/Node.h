#pragma once

#include <glm/glm.hpp>
#include "Pyxis/Renderer/Texture.h"
#include "Pyxis/Core/Timestep.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

namespace Pyxis
{
	class Node : public std::enable_shared_from_this<Node>
	{
	public:
		inline static uint32_t NodeCounter = 1;

		inline static std::unordered_map<uint32_t, Node*> Nodes;
		
		Node(const std::string& name = "Node");
		virtual ~Node();

		virtual void OnUpdate(Timestep ts) = 0;
		virtual void OnRender() = 0;
		virtual void InspectorRender();

		uint32_t GetID() { return m_ID; }

		void AddChild(const Ref<Node>& child);
		void RemoveChild(const Ref<Node>& child);
		
		/// <summary>
		/// Gets this transform 
		/// </summary>
		/// <returns></returns>
		glm::mat4 GetWorldTransform();

		void ResetLocalTransform();
		void SetLocalTransform(const glm::mat4& transform);
		void SetLocalTransformTest(const glm::mat4& transform);
		glm::mat4& GetLocalTransform();
		void Translate(glm::vec3 translation);
		void Rotate(glm::vec3 rotation);
		void Scale(glm::vec3 scale);

		std::string m_Name = "Node";
		Node* m_Parent = nullptr;
		std::vector<Ref<Node>> m_Children;

		bool m_Enabled = true;

	protected:

		/// <summary>
		/// Uses the private local position rotation and scale to set the local transform
		/// </summary>
		void UpdateLocalTransform();
		glm::mat4 m_LocalTransform = glm::mat4(1);
		glm::vec3 m_Position = glm::vec3(0);
		glm::vec3 m_Rotation = glm::vec3(0);
		glm::vec3 m_Scale = glm::vec3(1);

		const uint32_t m_ID = ++NodeCounter;
	};

	/// <summary>
	/// An object with a sprite
	/// </summary>
	/*class NodeWithSprite : public Node
	{
	public:
		NodeWithSprite(const std::string& name = "NodeWithSprite");
		NodeWithSprite(Ref<Texture2D> texture, const std::string& name = "NodeWithSprite");
		virtual  ~NodeWithSprite() = default;

		virtual void OnUpdate(Timestep ts) override;
		
		Ref<Texture2D> m_Texture;
	};*/

}