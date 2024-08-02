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
		bool ConnectToServer(const std::string& AddressAndPort);
		void ConnectionUpdate();
		void HandleMessages();
		void HandleTickClosure(MergedTickClosure& tc);
		bool CreateWorld();
		void TextCentered(std::string text);
		std::pair<float, float> GetMousePositionScene();
		

	public:
		void PaintBrushHologram();

	public:
		//things for the main menu to use to connect game world to server
		enum ConnectionStatus
		{
			NotConnected, Connecting, FailedToConnect, Disconnected, Connected
		};
		ConnectionStatus m_ConnectionStatus = NotConnected;
		std::string m_ConnectionErrorMessage = "";
	private:

		//game things
		Ref<World> m_World;
		std::chrono::time_point<std::chrono::steady_clock> m_UpdateTime = std::chrono::high_resolution_clock::now();
		float m_UpdatesPerSecond = 60;

		//core multiplayer things
		PixelClientInterface m_ClientInterface;
		std::deque<MergedTickClosure> m_MTCQueue;
		bool m_LatencyStateReset = false;
		uint64_t m_InputTick = 0;
		TickClosure m_CurrentTickClosure;
		std::deque<TickClosure> m_LatencyInputQueue;
		const int m_LatencyQueueLimit = 200; // this value should represent 1/60th the seconds server round trip delay
		uint64_t m_LatestMergedTick = 0;//temp?
		bool m_WaitingForOthers = false;

		//multiplayer connecting things
		
		bool m_WaitForWorldData = true;
		uint64_t m_TickToEnter = -1; // set to max value
		uint64_t m_TickToResetBox2D = -1; // set to max value

		struct PlayerCursor
		{
			PlayerCursor() = default;
			PlayerCursor(uint64_t id)
			{
				std::vector<glm::vec4> colorOptions =
				{
					glm::vec4(1,0,0,1),//red
					glm::vec4(1,0.5f,0,1),//orange
					glm::vec4(1,1,0,1),//yellow
					glm::vec4(0,1,0,1),//green
					glm::vec4(0,0,1,1),//blue
					glm::vec4(1,0,0.2f,1),//indigo
					glm::vec4(1,0,0.8f,1),//violet
				};
				color = colorOptions[id % colorOptions.size()];
			}
			glm::ivec2 pixelPosition = {0,0};
			glm::vec4 color = {1,1,1,1};
		};

		//multiplayer functionality
		std::unordered_map<uint64_t, PlayerCursor> m_PlayerCursors;

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
		BrushType m_BrushType = BrushType::circle;
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
