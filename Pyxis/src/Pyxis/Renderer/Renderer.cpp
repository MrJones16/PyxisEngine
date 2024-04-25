#include "pxpch.h"
#include "Renderer.h"
#include "Renderer2D.h"

#include "Platform/OpenGL/OpenGLShader.h"

namespace Pyxis
{
    Renderer::SceneData* Renderer::m_SceneData = new Renderer::SceneData;

    void Renderer::Init(uint32_t width, uint32_t height)
    {
        m_SceneData->Resolution = { width, height };
        RenderCommand::Init();
        Renderer2D::Init();
    }

    void Renderer::OnWindowResize(uint32_t width, uint32_t height)
    {
        RenderCommand::SetViewport(0, 0, width, height);
        //PX_CORE_INFO("Set viewport to: {0}, {1}", width, height);

        m_SceneData->Resolution = { width, height };

        for (auto& framebuffer : m_SceneData->FrameBuffers) {
            framebuffer->Resize(width, height);
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
        shader->SetMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
        shader->SetMat4("u_InverseViewProjection", glm::inverse(m_SceneData->ViewProjectionMatrix));
        shader->SetMat4("u_RotationMatrix", m_SceneData->RotationMatrix);
        shader->SetFloat3("u_CameraPosition", m_SceneData->CameraPosition);
        shader->SetFloat3("u_CameraRotation", m_SceneData->CameraRotation);
        shader->SetMat4("u_Transform", transform);
        shader->SetFloat2("u_Resolution", m_SceneData->Resolution);
        shader->SetFloat("u_FOV", m_SceneData->FOV);

        vertexArray->Bind();
        RenderCommand::DrawIndexed(vertexArray);
    }

    void Renderer::SubmitLine(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
    {
        shader->Bind();
        shader->SetMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
        shader->SetMat4("u_InverseViewProjection", glm::inverse(m_SceneData->ViewProjectionMatrix));
        shader->SetMat4("u_RotationMatrix", m_SceneData->RotationMatrix);
        shader->SetFloat3("u_CameraPosition", m_SceneData->CameraPosition);
        shader->SetFloat3("u_CameraRotation", m_SceneData->CameraRotation);
        shader->SetMat4("u_Transform", transform);
        shader->SetFloat2("u_Resolution", m_SceneData->Resolution);
        shader->SetFloat("u_FOV", m_SceneData->FOV);

        vertexArray->Bind();
        RenderCommand::DrawLines(vertexArray, 2);
    }

    void Renderer::PreBatch(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray)
    {
        shader->Bind();
        shader->SetMat4("u_ViewProjection", m_SceneData->ViewProjectionMatrix);
        shader->SetMat4("u_InverseViewProjection", glm::inverse(m_SceneData->ViewProjectionMatrix));
        shader->SetMat4("u_RotationMatrix", m_SceneData->RotationMatrix);
        shader->SetFloat3("u_CameraPosition", m_SceneData->CameraPosition);
        shader->SetFloat3("u_CameraRotation", m_SceneData->CameraRotation);
        shader->SetFloat2("u_Resolution", m_SceneData->Resolution);
        shader->SetFloat("u_FOV", m_SceneData->FOV);
        vertexArray->Bind();
    }

    void Renderer::SubmitBatch(const Ref<Shader>& shader, const Ref<VertexArray>& vertexArray, const glm::mat4& transform)
    {
        shader->SetMat4("u_Transform", transform);
        RenderCommand::DrawIndexed(vertexArray);
    }

}