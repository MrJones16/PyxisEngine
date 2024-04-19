#pragma once

#include "Pyxis.h"
#include "Pyxis/Core/OrthographicCameraController.h"

class Sandbox2D : public Pyxis::Layer
{
public:
	Sandbox2D();
	virtual ~Sandbox2D() = default;

	virtual void OnAttach();
	virtual void OnDetatch();

	virtual void OnUpdate(Pyxis::Timestep ts) override;
	virtual void OnImGuiRender() override;
	virtual void OnEvent(Pyxis::Event& e) override;
private:
	Pyxis::OrthographicCameraController m_OrthographicCameraController;

	Pyxis::Ref<Pyxis::FrameBuffer> m_FrameBuffer;

	glm::vec4 m_TestColor = { 0.2f, 0.3f, 0.8f , 1.0f};
};