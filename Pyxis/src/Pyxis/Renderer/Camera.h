#pragma once

#include "glm/glm.hpp"

namespace Pyxis
{
	class Camera
	{
	public:

		//Main Camera
		inline static Camera* s_MainCamera = nullptr;
		inline static Camera* Main() { return s_MainCamera; }
		Camera() { if (s_MainCamera == nullptr) { s_MainCamera = this; } }
		virtual ~Camera() { if (s_MainCamera == this) { s_MainCamera = nullptr; } }
		void SetMainCamera() { s_MainCamera = this; };

		//Camera Functions
		virtual void RecalculateProjectionMatrix() = 0;
		virtual const glm::mat4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

		virtual void RecalculateViewMatrix() = 0;
		virtual const glm::mat4& GetViewMatrix() const { return m_ViewMatrix; }

		virtual const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjectionMatrix; }

		//Position Functions
		virtual const glm::vec3& GetPosition() const = 0;
		virtual const glm::vec3& GetRotation() const = 0;
		virtual const glm::mat4& GetRotationMatrix() const = 0;

		//Camera Settings Functions
		virtual const float GetFOV() const { return m_FOV; }
		virtual void SetFOV(float fov) { m_FOV = fov; RecalculateProjectionMatrix(); };

		virtual const float GetAspect() const { return m_Aspect; }
		virtual void SetAspect(float aspect) {
			m_Aspect = aspect;
			m_Size.y = m_Size.x * aspect;
			RecalculateProjectionMatrix();
		}

		virtual const float GetNear() const { return m_Near; }
		virtual void SetNear(float nearClip) { m_Near = nearClip; RecalculateProjectionMatrix(); }

		virtual const float GetFar() const { return m_Far; }
		virtual void SetFar(float farClip) { m_Far = farClip; RecalculateProjectionMatrix(); }

		virtual const float GetWidth() const { return m_Size.x; }
		virtual void SetWidth(float width)
		{
			m_Size.x = width;
			if (m_LockAspect)
			{
				m_Size.y = m_Size.x * m_Aspect;
			}
			else
			{
				m_Aspect = m_Size.y / m_Size.x;
			}
			RecalculateProjectionMatrix(); 
		}

		virtual const float GetHeight() const { return m_Size.y; }
		virtual void SetHeight(float height)
		{
			m_Size.y = height;
			if (m_LockAspect)
			{
				m_Size.x = m_Size.y * (1 / m_Aspect);
			}
			else
			{
				m_Aspect = m_Size.y / m_Size.x;
			}
			RecalculateProjectionMatrix();
		}

		virtual glm::vec2 GetSize() const { return m_Size; }

	public:
		bool m_LockAspect = true;

	protected:
		glm::mat4 m_ProjectionMatrix = glm::mat4(1);
		glm::mat4 m_ViewMatrix = glm::mat4(1);
		glm::mat4 m_ViewProjectionMatrix = glm::mat4(1);

		glm::vec2 m_Size = glm::vec2(1);

		float m_Aspect = 9.0f / 16.0f;

		float m_FOV = 90.0f;
		float m_Near = -100.0f;
		float m_Far = 100.0f;
	};

	class PerspectiveCamera : public Camera
	{
	public:
		PerspectiveCamera(float aspect, float FOV, float nearClip, float farClip);

		virtual const glm::vec3& GetPosition() const override { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		virtual const glm::vec3& GetRotation() const override{ return m_Rotation; }
		void SetRotation(glm::vec3 rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }


		const float GetNear() const { return m_Near; }
		void SetNear(float nearClip) { m_Near = nearClip; RecalculateProjectionMatrix(); }

		const float GetFar() const { return m_Far; }
		void SetFar(float farClip) { m_Far = farClip; RecalculateProjectionMatrix(); }


		virtual const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		virtual const glm::mat4& GetViewMatrix() const override { return m_ViewMatrix; }
		virtual const glm::mat4& GetViewProjectionMatrix() const override { return m_ViewProjectionMatrix; }
		virtual const glm::mat4& GetRotationMatrix() const override { return m_RotationMatrix; }

	private:
		void RecalculateProjectionMatrix() override;
		void RecalculateViewMatrix() override;
	private:
		//glm::mat4 m_ProjectionMatrix;
		//glm::mat4 m_ViewMatrix;
		//glm::mat4 m_ViewProjectionMatrix;
		glm::mat3 m_RotationMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;
		
	};

	class OrthographicCamera : public Camera
	{
	public:
		OrthographicCamera(float width, float aspect, float nearClip, float farClip);
		//OrthographicCamera(float left, float right, float top, float bottom);

		virtual const glm::vec3& GetPosition() const override { return m_Position; }
		void SetPosition(const glm::vec3& position) { m_Position = position; RecalculateViewMatrix(); }

		virtual const glm::vec3& GetRotation() const override { return m_Rotation; }
		void SetRotation(glm::vec3 rotation) { m_Rotation = rotation; RecalculateViewMatrix(); }


		//void SetProjectionMatrix(float width, float height, ) const override { return m_ProjectionMatrix; }

		virtual const glm::mat4& GetProjectionMatrix() const override { return m_ProjectionMatrix; }
		virtual const glm::mat4& GetViewMatrix() const override  { return m_ViewMatrix; }
		virtual const glm::mat4& GetViewProjectionMatrix() const override  { return m_ViewProjectionMatrix; }
		virtual const glm::mat4& GetRotationMatrix() const override { return m_RotationMatrix; }


	private:
		void RecalculateProjectionMatrix() override;
		void RecalculateViewMatrix() override;
	private:
		//glm::mat4 m_ProjectionMatrix;
		//glm::mat4 m_ViewMatrix;
		//glm::mat4 m_ViewProjectionMatrix;
		glm::mat3 m_RotationMatrix;

		glm::vec3 m_Position;
		glm::vec3 m_Rotation;

	};
}