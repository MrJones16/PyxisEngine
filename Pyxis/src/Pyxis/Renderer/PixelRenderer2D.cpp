#include "Pyxis/Core/Application.h"
#include "Pyxis/Renderer/Renderer2D.h"
#include <Pyxis/Core/Input.h>
#include <Pyxis/Renderer/PixelRenderer2D.h>
#include <Pyxis/Renderer/RenderCommand.h>

namespace Pyxis {

PixelRenderer2DData PixelRenderer2D::s_RenderData = PixelRenderer2DData();

void PixelRenderer2D::Init() {

    Renderer2D::Init();

    glm::vec2 PaddedRenderResolution =
        s_RenderData.RenderResolution +
        (2 * s_RenderData.RenderResolutionPadding);

    FrameBufferSpecification deferredGBufferSpec;
    deferredGBufferSpec.Attachments = {
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // position 0
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // normal 1
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // albedo 2
        {FrameBufferTextureFormat::R32UI,
         FrameBufferTextureType::Color}, // node id 3
        {FrameBufferTextureFormat::Depth, FrameBufferTextureType::Depth}};
    deferredGBufferSpec.Width = s_RenderData.RenderResolution.x +
                                (s_RenderData.RenderResolutionPadding * 2);
    deferredGBufferSpec.Height = s_RenderData.RenderResolution.y +
                                 (s_RenderData.RenderResolutionPadding * 2);
    s_RenderData.DeferredGBuffer = FrameBuffer::Create(deferredGBufferSpec);

    FrameBufferSpecification lightingPassBufferSpec;
    lightingPassBufferSpec.Attachments = {
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // Color 0
        {FrameBufferTextureFormat::Depth, FrameBufferTextureType::Depth}};
    lightingPassBufferSpec.Width = PaddedRenderResolution.x;
    lightingPassBufferSpec.Height = PaddedRenderResolution.y;
    s_RenderData.DeferredLightingBuffer =
        FrameBuffer::Create(lightingPassBufferSpec);

    FrameBufferSpecification ParallaxBufferSpec;
    ParallaxBufferSpec.Attachments = {
        {FrameBufferTextureFormat::RGBA8,
         FrameBufferTextureType::Color}, // Color 0
        {FrameBufferTextureFormat::Depth, FrameBufferTextureType::Depth}};
    ParallaxBufferSpec.Width = PaddedRenderResolution.x;
    ParallaxBufferSpec.Height = PaddedRenderResolution.y;
    s_RenderData.ParallaxBuffer = FrameBuffer::Create(ParallaxBufferSpec);
}
void PixelRenderer2D::Shutdown() {
    // set renderdata to defaults which clears refs.
    s_RenderData = PixelRenderer2DData();
    Renderer2D::Shutdown();
}

void PixelRenderer2D::BeginFrame(PixelCameraNode *PixelCamera) {
    /////////////////////////////////////////////////////
    ///  Begin with fetching camera & resolution data
    /////////////////////////////////////////////////////
    s_RenderData.DisplayResolution =
        glm::vec2(Application::Get().GetWindow().GetWidth(),
                  Application::Get().GetWindow().GetHeight());
    if (!PixelCamera) {
        // Camera is null, or not a pixel camera!
        PX_CORE_ERROR("PixelRenderer2D was not passed a Pixel Camera!");
    } else {
        // Camera is a pixel camera. We need to check if the
        // size/padding has changed so that we can adjust the
        // framebuffers
        if (s_RenderData.RenderResolution != PixelCamera->GetSize() ||
            s_RenderData.RenderResolutionPadding !=
                PixelCamera->GetRenderResolutionPadding()) {

            s_RenderData.RenderResolution = PixelCamera->GetSize();
            s_RenderData.RenderResolutionPadding =
                PixelCamera->GetRenderResolutionPadding();

            glm::vec2 PaddedRenderResolution =
                s_RenderData.RenderResolution +
                (2 * s_RenderData.RenderResolutionPadding);

            // Camera size changed. Resize the buffers!
            s_RenderData.DeferredGBuffer->Resize(PaddedRenderResolution);
            s_RenderData.DeferredLightingBuffer->Resize(PaddedRenderResolution);
            s_RenderData.UpdateCache();
        }
    }
    s_RenderData.PixelCamera = PixelCamera;
    s_RenderData.PixelCamera->RecalculateViewMatrix();

    // Clear previous output
    RenderCommand::SetClearColor({0, 0, 0, 0});
    RenderCommand::Clear();

    Renderer2D::BeginScene(PixelCamera, s_RenderData.DeferredGBuffer,
                           s_RenderData.DeferredLightingBuffer);
}

void PixelRenderer2D::BeginSimulationPass() {
    // clear G buffer
    s_RenderData.DeferredGBuffer->Bind();
    RenderCommand::Clear();
    // now ready to draw to G buffer!
}
void PixelRenderer2D::EndSimulationPass() {
    Renderer2D::EndScene();
    // Mouse position is weird in this setup, as the output / buffers/ game are
    // all kinda doing their own thing. I have to find the mouse position from
    // 0-output, which 0 may start offset from the bottom left corner! check
    // RenderingThoughts.png for an analysis, where x is that offset.
    glm::vec2 mp = Input::GetMousePosition();
    // flip the y so bottom left is 0,0
    mp.y = s_RenderData.DisplayResolution.y - mp.y;

    glm::vec2 offsetForMouse =
        (s_RenderData.DisplayResolution - s_RenderData.TrueOutputSize) / 2.0f;
    mp -= offsetForMouse;
    mp /= s_RenderData.DisplayToRenderRatio;

    s_RenderData.DeferredGBuffer->Bind();
    if (mp.x >= 0 &&
        mp.x < (s_RenderData.TrueOutputSize.x /
                s_RenderData.DisplayToRenderRatio.x) &&
        mp.y >= 0 &&
        mp.y < (s_RenderData.TrueOutputSize.y /
                s_RenderData.DisplayToRenderRatio.y)) {
        UUID nodeID;
        s_RenderData.DeferredGBuffer->ReadPixel(3, mp.x, mp.y, &nodeID);
        Node::s_HoveredNodeID = nodeID;
        // PX_CORE_TRACE("Mouse Position: ({0},{1})", mp.x, mp.y);
        // PX_CORE_TRACE("Read Pixel: {0}", (uint32_t)nodeID);
    }
    s_RenderData.DeferredGBuffer->Unbind();
    Renderer2D::DrawDeferredLightingPass();
}

void PixelRenderer2D::BeginParallaxPass() {}
void PixelRenderer2D::DrawParallaxLayer(float Depth, float Distance,
                                        Ref<FrameBuffer> buffer) {
    // PX_CORE_TRACE("Render Resolution: ({0},{1})",
    //               s_RenderData.RenderResolution.x,
    //               s_RenderData.RenderResolution.y);
    // PX_CORE_TRACE("Padded Render Resolution: ({0},{1})",
    //               s_RenderData.PaddedRenderResolution.x,
    //               s_RenderData.PaddedRenderResolution.y);
    // PX_CORE_TRACE("Display Resolution: ({0},{1})",
    //               s_RenderData.DisplayResolution.x,
    //               s_RenderData.DisplayResolution.y);
    // PX_CORE_TRACE("Output Resolution: ({0},{1})",
    // s_RenderData.TrueOutputSize.x,
    //               s_RenderData.TrueOutputSize.y);

    glm::vec2 scaleToPixelPerfect =
        (s_RenderData.TrueOutputSize / s_RenderData.DisplayResolution);

    glm::vec2 offset = s_RenderData.PixelCamera->GetOffsetToGrid();
    offset /= s_RenderData.PaddedRenderResolution; // 0/642 - 1/642 ... how much
                                                   // of a pixel we shifted
    offset *= scaleToPixelPerfect; // offset depends on scale too

    Renderer2D::DrawScreenQuad(
        buffer->GetColorAttachmentRendererID(0), scaleToPixelPerfect,
        offset * 2.0f); // offset * 2.0f - glm::vec2(0, 0)
}
void PixelRenderer2D::EndParallaxPass() {}

void PixelRenderer2D::BeginPostEffectsPass() {}
void PixelRenderer2D::EndPostEffectsPass() {}

void PixelRenderer2D::BeginHUDPass() {}
void PixelRenderer2D::EndHUDPass() {}

} // namespace Pyxis
