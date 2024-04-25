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
	bool OnWindowResizeEvent(Pyxis::WindowResizeEvent& event);
private:
	Pyxis::OrthographicCameraController m_OrthographicCameraController;

	struct ProfileResult
	{
		const char* Name;
		float Time;
	};

	std::vector<ProfileResult> m_ProfileResults;
	std::vector<float> m_ProfileAverageValue;
	std::vector<float> m_ProfileAverageValueStorage;
	int m_ProfileAverageCount = 100;

	Pyxis::Ref<Pyxis::FrameBuffer> m_SceneFrameBuffer;
	Pyxis::Ref<Pyxis::Texture2D> m_TestTexture;
	Pyxis::Ref<Pyxis::Texture2D> m_SpritesheetTexture;
	Pyxis::Ref<Pyxis::SubTexture2D> m_SubTextureTest;

	glm::vec4 m_TestColor = { 0.2f, 0.3f, 0.8f , 1.0f };
	glm::vec3 m_TestPosition = { 0.0f, 0.0f, 0.0f};
	glm::vec2 m_TestSize = { 1.0f, 1.0f };
	float m_TestRotation = 0.0f;
};