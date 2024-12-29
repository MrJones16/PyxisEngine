#pragma once

#include <glm/glm.hpp>
#include "Pyxis/Renderer/Texture.h"
#include "Pyxis/Core/Timestep.h"
#include "imgui.h"
#include <glm/gtc/type_ptr.hpp>

namespace Pyxis
{
	class Node
	{
	public:
		inline static uint32_t NodeCounter;
		
		Node(const std::string& name = "Node");

		virtual void OnUpdate(Timestep ts);
		virtual void OnRender();
		virtual void InspectorRender();

		uint32_t GetID() { return m_ID; }

		void AddChild(const Ref<Node>& child);
		void RemoveChild(const Ref<Node>& child);
		
		/// <summary>
		/// Gets this transform 
		/// </summary>
		/// <returns></returns>
		glm::mat4 GetWorldTransform();
		glm::mat4 m_LocalTransform = glm::mat4(1);
		void Translate(glm::vec3 translation);
		void Rotate(glm::vec3 rotation);
		void Scale(glm::vec3 scale);

		std::string m_Name = "Node";
		Node* m_Parent = nullptr;
		std::vector<Ref<Node>> m_Children;

		bool m_Enabled = true;

	private:

		/// <summary>
		/// Uses the private local position rotation and scale to set the local transform
		/// </summary>
		void UpdateLocalTransform();
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