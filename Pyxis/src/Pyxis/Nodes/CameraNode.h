#pragma once

//#include <Pyxis/Renderer/Renderer2D.h>
//#include <glm/gtc/matrix_transform.hpp>
#include <Pyxis/Nodes/Node3D.h>

#include <Pyxis/Renderer/Camera.h>
#include <Pyxis/Core/Application.h>

namespace Pyxis
{
	
	//Camera node: inherits a camera and node.
	class CameraNode : public Node3D, public Camera
	{
	public:
		CameraNode(const std::string& name = "CameraNode") : Node3D(name), Camera() { RecalculateProjectionMatrix(); };
		CameraNode(UUID id) : Node3D(id), Camera() {};
		virtual ~CameraNode() = default;

		//Serialization
		void Serialize(json& j) override;
		void Deserialize(json& j) override;

		//Functions for game usage
		glm::vec2 MouseToWorldPos(glm::vec2 mousePos);

		//functions for this
		void RecalculateProjectionMatrix();

		//overrides for camera class
		virtual void RecalculateViewMatrix() override;
		inline virtual const glm::vec3& GetPosition() const override { return m_Position; };
		inline virtual const glm::vec3& GetRotation() const override { return m_Rotation; };
		virtual const glm::mat4& GetRotationMatrix() const override;

	};
	
	REGISTER_SERIALIZABLE_NODE(CameraNode);

}