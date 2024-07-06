#pragma once

#include "Pyxis.h"
#include "Pyxis/Game/Scene.h"
#include "Pyxis/Core/OrthographicCameraController.h"
#include "Panels/Panel.h"
#include "Panels/ProfilingPanel.h"

#include "common/World.h"
#include "common/PixelClientInterface.h"



namespace Pyxis
{
	class GameLayer : public Layer
	{
	public:
		GameLayer();
		virtual ~GameLayer();

		//Layer functions
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		bool OnWindowResizeEvent(WindowResizeEvent& event);
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		bool OnMouseScrolledEvent(MouseScrolledEvent& event);

		//game functions
		void HandleMessages();
		bool CreateWorld();
		void TextCentered(std::string text);
		std::pair<float, float> GetMousePositionScene();
		

	public:
		enum BrushType {
			circle = 0, square = 1, end
		};
		void PaintElementAtCursor(glm::ivec2 pixelPos);
		void PaintBrushHologram();

	private:
		//game things
		Ref<World> m_World;
		std::chrono::time_point<std::chrono::steady_clock> m_UpdateTime = std::chrono::high_resolution_clock::now();
		float m_UpdatesPerSecond = 60;

		//multiplayer things
		PixelClientInterface m_ClientInterface;
		std::deque<MergedTickClosure> m_MTCQueue;
		bool m_LatencyStateReset = false;
		bool m_Connecting = true;
		bool m_WaitForWorldData = true;
		uint64_t m_TickToEnter = -1; // set to max value
		uint64_t m_TickToResetBox2D = -1; // set to max value

		uint64_t m_InputTick = 0;
		TickClosure m_CurrentTickClosure;
		std::deque<TickClosure> m_LatencyInputQueue;
		const int m_LatencyQueueLimit = 200; // this value should represent 1/60th the seconds server round trip delay
		uint64_t latestMergedTick = 0;
		bool m_WaitingForOthers = false;


		//scene things
		Ref<FrameBuffer> m_SceneFrameBuffer;
		OrthographicCameraController m_OrthographicCameraController;
		glm::vec2 m_ViewportSize;
		ImVec2 m_ViewportOffset;

		bool m_SceneViewIsFocused = false;

		//Tools / Panels
		std::vector<Ref<Panel>> m_Panels;
		Ref<ProfilingPanel> m_ProfilingPanel;

		//player tools
		int m_SelectedElementIndex = 0;
		float m_BrushSize = 1;
		int m_BrushType = BrushType::circle;
		Element m_HoveredElement = Element();
		bool m_BuildingRigidBody = false;

		glm::ivec2 m_RigidMin = { 99999999, 99999999 };
		glm::ivec2 m_RigidMax = { -99999999 , -99999999 };

		//testing / game
		bool m_SimulationRunning = false;
		bool m_Hovering = false;
		float m_DouglasThreshold = 1.0f;
	};
}
