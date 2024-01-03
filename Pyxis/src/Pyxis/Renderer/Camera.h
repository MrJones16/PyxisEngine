#pragma once

#include "glm/glm.hpp"

namespace Pyxis
{
	class Camera
	{
	public:
		virtual const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }
		virtual const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }
		virtual const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		virtual const glm::vec3& GetPosition() const = 0;
		virtual const glm::vec3& GetRotation() const = 0;
		virtual const glm::mat3& GetRotationMatrix() const = 0;
		virtual const float GetFOV() const = 0;
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;
	};

	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(float aspect, float FOV, float nearClip, float farClip);

		virtual const glm::vec3& GetPosition() const override { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		virtual const glm::vec3& GetRotation() const override{ return m_Rotation; }
		void SetRotation(glm::vec3 rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const float GetFOV() const override { return m_FOV; }
		void SetFOV(float fov) { m_FOV = fov; RecalculateProjectionMatrix(); }

		const float GetAspect() const { return m_Aspect; }
		void SetAspect(float aspect) { m_Aspect = aspect; RecalculateProjectionMatrix(); }

		const float GetNear() const { return m_Near; }
		void SetNear(float nearClip) { m_Near = nearClip; RecalculateProjectionMatrix(); }

		const float GetFar() const { return m_Far; }
		void SetFar(float farClip) { m_Far = farClip; RecalculateProjectionMatrix(); }


		virtual const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		virtual const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		virtual const glm::mat4& GetViewProjectionMatrix() const override { return m_ViewProjectionMatrix; }
		virtual const glm::mat3& GetRotationMatrix() const override { return m_RotationMatrix; }

	private:
		void RecalculateProjectionMatrix();
		void RecalculateViewMatrix();
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;
		glm::mat3 m_RotationMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		

		float m_FOV;
		float m_Aspect;
		float m_Near = -100.0f;
		float m_Far = 100.0f;
	};

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float width, float height, float nearClip, float farClip);
		//OrthographicCamera(float left, float right, float top, float bottom);

		virtual const glm::vec3& GetPosition() const override { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		virtual const glm::vec3& GetRotation() const override { return m_Rotation; }
		void SetRotation(glm::vec3 rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }

		const float GetWidth() const { return m_Width; }
		void SetWidth(float width) { m_Width = width; RecalculateProjectionMatrix(); }

		const float GetHeight() const { return m_Height; }
		void SetHeight(float height) { m_Height = height; RecalculateProjectionMatrix(); }

		const float GetNear() const { return m_Near; }
		void SetNear(float nearClip) { m_Near = nearClip; RecalculateProjectionMatrix(); }

		const float GetFar() const { return m_Far; }
		void SetFar(float farClip) { m_Far = farClip; RecalculateProjectionMatrix(); }


		virtual const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		virtual const glm::mat4& GetViewMatrix() const override  { return m_ViewMatrix; }
		virtual const glm::mat4& GetViewProjectionMatrix() const override  { return m_ViewProjectionMatrix; }

	private:
		void RecalculateProjectionMatrix();
		void RecalculateViewMatrix();
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix;
		glm::mat4 m_ViewProjectionMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;

		float m_Width;
		float m_Height;
		float m_Near = -100.0f;
		float m_Far = 100.0f;
	};
}