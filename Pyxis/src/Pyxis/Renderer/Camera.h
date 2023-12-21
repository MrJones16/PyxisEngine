#pragma once

#include "glm/glm.hpp"

namespace Pyxis
{
	class OrthographicCamera
	{
	public:
		OrthographicCamera(float width, float height, float nearClip, float farClip);
		//OrthographicCamera(float left, float right, float top, float bottom);

		const glm::vec3& GetPosition() const { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		const float GetRotation() const { return m_Rotation; }
		void SetRotation(float rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const float GetWidth() const { return m_Width; }
		void SetWidth(float width) { m_Width = width; RecalculateProjectionMatrix(); }

		const float GetHeight() const { return m_Height; }
		void SetHeight(float height) { m_Height = height; RecalculateProjectionMatrix(); }

		const float GetNear() const { return m_Near; }
		void SetNear(float nearClip) { m_Near = nearClip; RecalculateProjectionMatrix(); }

		const float GetFar() const { return m_Far; }
		void SetFar(float farClip) { m_Far = farClip; RecalculateProjectionMatrix(); }


		const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

	private:
		void RecalculateProjectionMatrix();
		void RecalculateViewMatrix();
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position;
		float m_Rotation = 0.0f;

		float m_Width;
		float m_Height;
		float m_Near = -100.0f;
		float m_Far = 100.0f;
	};
}