#pragma once

#include "Pyxis.h"
#include "Pyxis/Game/Scene.h"
#include "Pyxis/Core/OrthographicCameraController.h"
#include "Panels/Panel.h"


namespace Pyxis
{
	class EditorLayer : public Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;

		virtual void OnAttach();
		virtual void OnDetatch();

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
		bool OnWindowResizeEvent(WindowResizeEvent& event);
	private:
		//scene things
		OrthographicCameraController m_OrthographicCameraController;
		glm::vec2 m_ViewportSize;
		Ref<Scene> m_ActiveScene;

		bool m_SceneViewIsFocused = false;

		//Tools / Panels
		std::vector<Ref<Panel>> m_Panels;

		struct ProfileResult
		{
			const char* Name;
			float Time;
		};

		std::vector<ProfileResult> m_ProfileResults;
		std::vector<float> m_ProfileAverageValue;
		std::vector<float> m_ProfileAverageValueStorage;
		int m_ProfileAverageCount = 100;

		Ref<FrameBuffer> m_SceneFrameBuffer;
		Ref<Texture2D> m_TestTexture;
		//Ref<Texture2D> m_SpritesheetTexture;
		//Ref<SubTexture2D> m_SubTextureTest;

		glm::vec4 m_TestColor = { 0.2f, 0.3f, 0.8f , 1.0f };
		glm::vec3 m_TestPosition = { 0.0f, 0.0f, 0.0f };
		glm::vec2 m_TestSize = { 1.0f, 1.0f };
		float m_TestRotation = 0.0f;
	};
}
