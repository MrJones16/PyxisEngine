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
		virtual ~GameLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		std::pair<float, float> GetMousePositionScene();
		bool OnWindowResizeEvent(WindowResizeEvent& event);
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		bool OnMouseScrolledEvent(MouseScrolledEvent& event);

		enum BrushType {
			circle = 0, square = 1, end
		};

	private:

		void PaintElementAtCursor();
		void PaintBrushHologram();
		//game things
		Ref<World> m_World;
		std::chrono::time_point<std::chrono::steady_clock> m_UpdateTime = std::chrono::high_resolution_clock::now();
		float m_UpdatesPerSecond = 60;
		//Ref<Chunk> m_Chunk;

		//scene things
		Ref<FrameBuffer> m_SceneFrameBuffer;
		OrthographicCameraController m_OrthographicCameraController;
		glm::vec2 m_ViewportSize;
		ImVec2 m_ViewportOffset;
		Ref<Scene> m_ActiveScene;

		bool m_SceneViewIsFocused = false;

		//Tools / Panels
		std::vector<Ref<Panel>> m_Panels;
		Ref<ProfilingPanel> m_ProfilingPanel;

		//player tools
		int m_SelectedElementIndex = 0;
		float m_BrushSize = 1;
		int m_BrushType = BrushType::circle;
		Element m_HoveredElement = Element();

		glm::ivec2 m_RigidMin = { 99999999, 99999999 };
		glm::ivec2 m_RigidMax = { -99999999 , -99999999 };

		//testing / game
		bool m_SimulationRunning = false;
		bool m_Hovering = false;
	};
}
