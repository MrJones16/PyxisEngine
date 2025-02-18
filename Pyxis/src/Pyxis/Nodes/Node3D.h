#pragma once

#include <Pyxis/Nodes/Node.h>

namespace Pyxis
{
	class Node3D : public Node
	{
	protected:
		/// <summary>
		/// Uses the private local position rotation and scale to set the local transform
		/// </summary>
		glm::mat4 m_LocalTransform = glm::mat4(1);
		glm::vec3 m_Position = glm::vec3(0);
		glm::vec3 m_Rotation = glm::vec3(0);
		glm::vec3 m_Scale = glm::vec3(1);


	public:

		Node3D(const std::string& name) : Node(name)
		{

		}

		Node3D(UUID id) : Node(id)
		{

		}

		void OnInspectorRender() override;

		void Serialize(json& j) override;
		void Deserialize(json& j) override;
		

		/// <summary>
		/// Gets this transform 
		/// </summary>
		/// <returns></returns>
		virtual glm::mat4 GetWorldTransform();

		virtual void ResetLocalTransform();
		glm::mat4& GetLocalTransform();
		virtual void SetLocalTransform(const glm::mat4& transform);
		virtual void UpdateLocalTransform();

		virtual void Translate(const glm::vec3& translation);
		virtual void SetPosition(const glm::vec3& position);

		virtual void Rotate(const glm::vec3& rotation);
		virtual void SetRotation(const glm::vec3& rotation);

		virtual void Scale(const glm::vec3& scale);
		virtual void SetScale(const glm::vec3& scale);
	};

	REGISTER_SERIALIZABLE_NODE(Node3D);
}
