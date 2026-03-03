#pragma once

#include <Pyxis/Core/Layer.h>

namespace Pyxis {
class UILayer : public Layer {
  public:
    UILayer();
    ~UILayer();

    virtual void OnAttach() override;
    virtual void OnDetach() override;
    virtual void OnUIUpdate() override;
    virtual void OnEvent(Event &event) override;

    void BeginFrame(glm::vec2 screenSize, glm::vec2 mousePosition, bool lmb,
                    glm::vec2 mouseWheel, float deltaTime);
    void EndFrame();
};
} // namespace Pyxis
