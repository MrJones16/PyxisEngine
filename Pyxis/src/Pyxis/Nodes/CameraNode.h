#pragma once

//#include <Pyxis/Renderer/Renderer2D.h>
//#include <glm/gtc/matrix_transform.hpp>
#include <Pyxis/Nodes/Node.h>

#include <Pyxis/Renderer/Camera.h>
#include <Pyxis/Core/Application.h>

namespace Pyxis
{
	
	//Camera node: inherits a camera and node.
	class CameraNode : public Node, public Camera
	{
	public:
		CameraNode(const std::string& name = "CameraNode") : Node(name), Camera() { RecalculateProjectionMatrix(); };
		virtual ~CameraNode() = default;


		//Functions for game usage
		glm::vec2 MouseToWorldPos(glm::vec2 mousePos);

		//functions for this
		void RecalculateProjectionMatrix();

		//overrides for camera class
		virtual void RecalculateViewMatrix() override;
		inline virtual const glm::vec3& GetPosition() const override { return m_Position; };
		inline virtual const glm::vec3& GetRotation() const override { return m_Rotation; };
		virtual const glm::mat4& GetRotationMatrix() const override;

		//override transform changing to affect camera matrices
		/*virtual void ResetLocalTransform() override;
		virtual void SetLocalTransform(const glm::mat4& transform) override;
		virtual void Translate(glm::vec3 translation) override;
		virtual void Rotate(glm::vec3 rotation) override;
		virtual void Scale(glm::vec3 scale) override;*/

	};
	

}