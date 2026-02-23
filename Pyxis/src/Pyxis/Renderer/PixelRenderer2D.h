#pragma once

#include "Pyxis/Nodes/PixelCameraNode.h"
#include "Pyxis/Renderer/FrameBuffer.h"
#include <Pyxis/Renderer/Renderer2D.h>
#include <glm/fwd.hpp>

namespace Pyxis {

struct ParallaxLayer {
  public:
    ParallaxLayer(Ref<FrameBuffer> FrameBuffer, float Depth, float Distance) {
        Buffer = FrameBuffer;
        this->Depth = Depth;
        this->Distance = Distance;
    }

    Ref<FrameBuffer> Buffer;
    float Depth = 0;
    float Distance = 0;
};

struct PixelRenderer2DData {
  public:
    // Store general data & camera
    PixelCameraNode *PixelCamera = nullptr;
    glm::vec2 RenderResolution = glm::vec2(480, 270);
    float RenderResolutionPadding = 128;
    glm::vec2 DisplayResolution = glm::vec2(1920, 1080);

    // Cache ratios and vecs needed later
    glm::vec2 PaddedRenderResolution =
        RenderResolution + (2 * RenderResolutionPadding);
    glm::vec2 DisplayToRenderRatio = DisplayResolution / RenderResolution;
    glm::vec2 TrueOutputSize = PaddedRenderResolution * DisplayToRenderRatio;

    void UpdateCache() {
        PaddedRenderResolution =
            RenderResolution + (2 * RenderResolutionPadding);
        DisplayToRenderRatio = DisplayResolution / RenderResolution;
        TrueOutputSize = PaddedRenderResolution * DisplayToRenderRatio;
    }

    // Data needed for simulation pass
    Ref<FrameBuffer> DeferredGBuffer;
    Ref<FrameBuffer> DeferredLightingBuffer;

    // Data needed for Parallax Pass
    Ref<FrameBuffer> ParallaxBuffer;

    // Data needed for PostEffects Pass

    // Data needed for HUD Pass
};

// Pixel renderer 2d will be the API to interact with adding parallax layers,
// post effects, HUD, and more.
class PixelRenderer2D {
  public:
    static PixelRenderer2DData s_RenderData;

    // Create the framebuffers
    static void Init();
    static void Shutdown();

    // Called at the beginning of the rendering sequence to initialize the
    // camera & resolution info. This will resize buffers if needed.
    // This also clears the previous frame
    static void BeginFrame(PixelCameraNode *PixelCamera);

    //////////////////////////////////
    ///  Simulation Pass
    //////////////////////////////////
    /// Drawing game objects /nodes with their proper shaders
    ///
    /// In Buffer: None
    /// Out Buffer: Framebuffer made from Deferred G and then lighting
    /// Resolution: Pixel Resolution
    static void BeginSimulationPass();
    static void EndSimulationPass();

    //////////////////////////////////
    ///  Parallax Pass
    //////////////////////////////////
    /// Drawing multiple layers together
    ///
    /// In Buffer: List of "Parallax Layers" with their depth, distance, and
    /// texture Out Buffer: Framebuffer with all layers added together.
    /// Resolution: Pixel Resolution -> Display Resolution (Scaling and offset
    /// done here!)
    static void BeginParallaxPass();
    static void DrawParallaxLayer(float Depth, float Distance,
                                  Ref<FrameBuffer> buffer);
    static void EndParallaxPass();

    //////////////////////////////////
    ///  Post-Effects Pass
    //////////////////////////////////
    /// Bloom, Blur, Distortion, Etc.
    ///
    /// In Buffer: Compiled Parallax Buffer
    /// Out Buffer: Straight to output buffer
    /// Resolution: Pixel Resolution
    static void BeginPostEffectsPass();
    static void EndPostEffectsPass();

    //////////////////////////////////
    ///  HUD Pass
    //////////////////////////////////
    /// Drawing HUD & UI to screen
    ///
    /// In Buffer: None
    /// Out Buffer: Output Buffer
    /// Resolution: Display Resolution
    static void BeginHUDPass();
    static void EndHUDPass();

  private:
};

} // namespace Pyxis
