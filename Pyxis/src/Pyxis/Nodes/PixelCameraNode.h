#pragma once

#include "CameraNode.h"

namespace Pyxis
{
	
	//Camera node: inherits a camera and node.
	class PixelCameraNode : public CameraNode
	{
	public:
		PixelCameraNode(const std::string& name = "PixelCameraNode") : CameraNode() { RecalculateProjectionMatrix(); };
		PixelCameraNode(UUID id) : CameraNode(id) {};
		virtual ~PixelCameraNode() = default;

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
		virtual const glm::mat3 GetRotationMatrix() const override;

	};
	
	REGISTER_SERIALIZABLE_NODE(PixelCameraNode);

}