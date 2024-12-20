#pragma once

#include "RenderCommand.h"
#include "Camera.h"
#include "Shader.h"
#include "FrameBuffer.h"

namespace Pyxis
{
	class Renderer
	{
	public:
		static void Init(uint32_t width, uint32_t height);
		static void OnWindowResize(uint32_t width, uint32_t height);

		static void BeginScene(Camera& camera); // TODO: lights, environment, etc
		static void EndScene();

		static void AddFrameBuffer(Ref<FrameBuffer> frameBuffer);

		static void Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));
		static void SubmitLine(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));
		static void PreBatch(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray);
		static void SubmitBatch(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform = glm::mat4(1.0f));

		inline static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
	private:
		struct SceneData
		{
			glm::mat4 ViewProjectionMatrix;
			glm::mat4 RotationMatrix;
			glm::vec3 CameraRotation;
			glm::vec3 CameraPosition;
			float FOV;
			glm::vec2 Resolution;
			std::vector<Ref<FrameBuffer>> FrameBuffers;
		};
		static SceneData* m_SceneData;
	};

}