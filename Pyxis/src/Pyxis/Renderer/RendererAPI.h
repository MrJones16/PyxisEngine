#pragma once

#include "VertexArray.h"
#include <glm/glm.hpp>

namespace Pyxis {
class RendererAPI {
  public:
    enum class API { None = 0, OpenGL = 1 };

  public:
    virtual void Init() = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width,
                             uint32_t height) = 0;

    virtual void SetClearColor(const glm::vec4 &color) = 0;
    virtual void Clear() = 0;

    virtual void EnableDepthTesting() = 0;
    virtual void DisableDepthTesting() = 0;

    virtual void EnableBlending() = 0;
    virtual void DisableBlending() = 0;
    enum BlendFactor {
        ZERO,
        ONE,
        SRC_COLOR,
        ONE_MINUS_SRC_COLOR,
        DST_COLOR,
        ONE_MINUS_DST_COLOR,
        SRC_ALPHA,
        ONE_MINUS_SRC_ALPHA,
        DST_ALPHA,
        ONE_MINUS_DST_ALPHA

    };
    virtual void SetBlendFactors(BlendFactor srcFactor,
                                 BlendFactor dstFactor) = 0;

    virtual void DrawIndexed(const Ref<VertexArray> &VertexArray,
                             uint32_t indexCount = 0) = 0;
    virtual void DrawArray(const Ref<VertexArray> &VertexArray) = 0;
    virtual void DrawLines(const Ref<VertexArray> &VertexArray,
                           uint32_t VertexCount) = 0;

    virtual void BindTexture2D(const uint32_t TextureID,
                               const uint32_t Unit) = 0;

    inline static API GetAPI() { return s_API; }

  private:
    static API s_API;
};
} // namespace Pyxis
