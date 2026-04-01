#pragma once

#include "CameraNode.h"

namespace Pyxis {

// Camera node: inherits a camera and node.
class PixelCameraNode : public CameraNode {
  public:
    PixelCameraNode(const std::string &name = "PixelCameraNode")
        : CameraNode() {
        RecalculateProjectionMatrix();
    };
    PixelCameraNode(UUID id) : CameraNode(id) {};
    virtual ~PixelCameraNode() = default;

    // Serialization
    void Serialize(json &j) override;
    void Deserialize(json &j) override;

    // Functions for game usage
    glm::vec2 MouseToWorldPos(glm::vec2 mousePos);

    // functions for this
    void RecalculateProjectionMatrix() override;

    // overrides for camera class
    virtual void RecalculateViewMatrix() override;
    inline virtual const glm::vec3 &GetPosition() const override {
        return m_Position;
    };
    inline virtual const glm::vec3 &GetRotation() const override {
        return m_Rotation;
    };

    virtual glm::vec2 GetSize() const override;

    virtual const float GetWidth() const override;
    virtual void SetWidth(float width) override;

    virtual const float GetHeight() const override;
    virtual void SetHeight(float height) override;

    virtual const glm::mat3 GetRotationMatrix() const override;

    inline virtual glm::vec2 GetOffsetToGrid() const { return m_OffsetToGrid; }

    inline virtual void SetRenderResolutionPadding(int PaddingDepthInPixels) {
        m_RenderResolutionPadding = PaddingDepthInPixels;
    }
    inline virtual float GetRenderResolutionPadding() const {
        return m_RenderResolutionPadding;
    }

  protected:
    // used for pixel perfect camera, track offset of camera to pixel grid to
    // shift the rendered screen.
    glm::vec3 m_OffsetToGrid = glm::vec3(0);

    // Depth of additional buffer, in pixels. 1 means a 1 px buffer around
    // entire buffer, so +2x,+2y.
    // adding 128 as I want lights to be able to have this range.
    float m_RenderResolutionPadding = 2;
};

REGISTER_SERIALIZABLE_NODE(PixelCameraNode);

} // namespace Pyxis
