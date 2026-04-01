#include "OpenGLRendererAPI.h"
#include "pxpch.h"

#include <glad/glad.h>

namespace Pyxis {
void OpenGLRendererAPI::Init() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);
}
void OpenGLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width,
                                    uint32_t height) {
    glViewport(x, y, width, height);
}
void OpenGLRendererAPI::SetClearColor(const glm::vec4 &color) {
    glClearColor(color.r, color.g, color.b, color.a);
}

void OpenGLRendererAPI::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLRendererAPI::EnableDepthTesting() { glEnable(GL_DEPTH_TEST); }

void OpenGLRendererAPI::DisableDepthTesting() { glDisable(GL_DEPTH_TEST); }

void OpenGLRendererAPI::EnableBlending() { glEnable(GL_BLEND); }
void OpenGLRendererAPI::DisableBlending() { glDisable(GL_BLEND); }
void OpenGLRendererAPI::SetBlendFactors(BlendFactor srcFactor,
                                        BlendFactor dstFactor) {
    GLenum src, dst = 0;
    switch (srcFactor) {
    case RendererAPI::ZERO:
        src = GL_ZERO;
        break;
    case RendererAPI::ONE:
        src = GL_ONE;
        break;
    case RendererAPI::SRC_COLOR:
        src = GL_SRC_COLOR;
        break;
    case RendererAPI::ONE_MINUS_SRC_COLOR:
        src = GL_ONE_MINUS_SRC_COLOR;
        break;
    case RendererAPI::DST_COLOR:
        src = GL_DST_COLOR;
        break;
    case RendererAPI::ONE_MINUS_DST_COLOR:
        src = GL_ONE_MINUS_DST_COLOR;
        break;
    case RendererAPI::SRC_ALPHA:
        src = GL_SRC_ALPHA;
        break;
    case RendererAPI::ONE_MINUS_SRC_ALPHA:
        src = GL_ONE_MINUS_SRC_ALPHA;
        break;
    case RendererAPI::DST_ALPHA:
        src = GL_DST_ALPHA;
        break;
    case RendererAPI::ONE_MINUS_DST_ALPHA:
        src = GL_ONE_MINUS_DST_ALPHA;
        break;
    default:
        break;
    }
    switch (dstFactor) {
    case RendererAPI::ZERO:
        dst = GL_ZERO;
        break;
    case RendererAPI::ONE:
        dst = GL_ONE;
        break;
    case RendererAPI::SRC_COLOR:
        dst = GL_SRC_COLOR;
        break;
    case RendererAPI::ONE_MINUS_SRC_COLOR:
        dst = GL_ONE_MINUS_SRC_COLOR;
        break;
    case RendererAPI::DST_COLOR:
        dst = GL_DST_COLOR;
        break;
    case RendererAPI::ONE_MINUS_DST_COLOR:
        dst = GL_ONE_MINUS_DST_COLOR;
        break;
    case RendererAPI::SRC_ALPHA:
        dst = GL_SRC_ALPHA;
        break;
    case RendererAPI::ONE_MINUS_SRC_ALPHA:
        dst = GL_ONE_MINUS_SRC_ALPHA;
        break;
    case RendererAPI::DST_ALPHA:
        dst = GL_DST_ALPHA;
        break;
    case RendererAPI::ONE_MINUS_DST_ALPHA:
        dst = GL_ONE_MINUS_DST_ALPHA;
        break;
    default:
        break;
    }

    glBlendFunc(src, dst);
}

void OpenGLRendererAPI::DrawIndexed(const Ref<VertexArray> &VertexArray,
                                    uint32_t indexCount) {
    uint32_t count =
        indexCount ? indexCount : VertexArray->GetIndexBuffer()->GetCount();
    glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRendererAPI::DrawArray(const Ref<VertexArray> &VertexArray) {
    glDrawArrays(GL_TRIANGLES, 0, 2);
}

void OpenGLRendererAPI::DrawLines(const Ref<VertexArray> &VertexArray,
                                  uint32_t VertexCount) {
    VertexArray->Bind();
    glDrawArrays(GL_LINES, 0, VertexCount);
}

void OpenGLRendererAPI::BindTexture2D(const uint32_t TextureID,
                                      const uint32_t Unit) {
    glBindTextureUnit(Unit, TextureID);
    // glBindTexture(GL_TEXTURE_2D, textureID);
}

} // namespace Pyxis
