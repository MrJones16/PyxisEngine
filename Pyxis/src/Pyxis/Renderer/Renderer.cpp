#include "pxpch.h"
#include "Renderer.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Pyxis
{
    Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

    void Renderer::Init(uint32_t width, uint32_t height)
    {
        m_SceneData->Resolution = { width, height };
        RenderCommand::Init();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
        PX_CORE_INFO("Set viewport to: {0}, {1}", width, height);

        m_SceneData->Resolution = { width, height };

        for (auto& framebuffer : m_SceneData->FrameBuffers) {
            framebuffer->RescaleFrameBuffer(width, height);
        }

    }

    void Renderer::BeginScene(Camera& camera)
    {
        m_SceneData->ViewProjectionMatrix = camera.GetViewProjectionMatrix();
        m_SceneData->CameraPosition = camera.GetPosition();
        m_SceneData->CameraRotation = camera.GetRotation();
        m_SceneData->RotationMatrix = camera.GetRotationMatrix();
        m_SceneData->FOV = 1 / glm::tan(glm::radians(camera.GetFOV() / 2));
    }

    void Renderer::EndScene()
    {
        
    }

    void Renderer::AddFrameBuffer(Ref<FrameBuffer> frameBuffer)
    {
        m_SceneData->FrameBuffers.push_back(frameBuffer);
    }

    void Renderer::Submit(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
    {
        shader->Bind();
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_RotationMatrix", m_SceneData->RotationMatrix);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat3("u_CameraPosition", m_SceneData->CameraPosition);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat3("u_CameraRotation", m_SceneData->CameraRotation);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformMat4("u_Transform", transform);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat2("u_Resolution", m_SceneData->Resolution);
        std::dynamic_pointer_cast<OpenGLShader>(shader)->UploadUniformFloat("u_FOV", m_SceneData->FOV);


        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }
}