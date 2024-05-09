#pragma once

#include "Pyxis.h"
#include "Pyxis/Game/Scene.h"
#include "Pyxis/Core/OrthographicCameraController.h"
#include "Panels/Panel.h"
#include "Panels/ProfilingPanel.h"

#include "World.h"


namespace Pyxis
{
	class GameLayer : public Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer() = default;

		virtual void OnAttach();
		virtual void OnDetatch();

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;
		bool OnWindowResizeEvent(WindowResizeEvent& event);
		bool OnKeyPressedEvent(KeyPressedEvent& event);
	private:
		//game things
		Ref<World> m_World;
		//Ref<Chunk> m_Chunk;

		//scene things
		OrthographicCameraController m_OrthographicCameraController;
		glm::vec2 m_ViewportSize;
		Ref<Scene> m_ActiveScene;

		bool m_SceneViewIsFocused = false;

		//Tools / Panels
		std::vector<Ref<Panel>> m_Panels;
		Ref<ProfilingPanel> m_ProfilingPanel;

		Ref<FrameBuffer> m_SceneFrameBuffer;
		Ref<Texture2D> m_TestTexture;
		//Ref<Texture2D> m_SpritesheetTexture;
		//Ref<SubTexture2D> m_SubTextureTest;

		//testing / game
		bool m_SimulationRunning = false;
	};
}
