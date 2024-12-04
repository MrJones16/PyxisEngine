#pragma once

#include "Pyxis.h"
#include "Pyxis/Game/Scene.h"
#include "Pyxis/Core/OrthographicCameraController.h"
#include "Pyxis/Core/Panel.h"
#include "Pyxis/Core/ProfilingPanel.h"
#include <Pyxis/Network/NetworkClient.h>

#include "World.h"



namespace Pyxis
{

	class GameLayer : public Layer, public Network::ClientInterface
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
		// 
		void StartSingleplayer();
		void StartMultiplayer(const std::string& AddressAndPort);
		void OnConnectionSuccess() override;
		//void ConnectionUpdate();
		void HandleMessages();
		void HandleTickClosure(MergedTickClosure& tc);
		bool CreateWorld();
		void TextCentered(std::string text);
		std::pair<float, float> GetMousePositionScene();
		

	public:
		void PaintBrushHologram();
		
	private:

		//game things
		Ref<World> m_World;
		std::chrono::time_point<std::chrono::high_resolution_clock> m_UpdateTime = std::chrono::high_resolution_clock::now();
		float m_UpdatesPerSecond = 30.0f;

		//core multiplayer things
		//multiplayer connecting things
		enum MultiplayerState
		{
			Singleplayer,
			Connecting,
			GatheringPlayerData,
			DownloadingWorld,
			CatchingUp,
			Connected,

		};
		MultiplayerState m_MultiplayerState = Singleplayer;
		TickClosure m_CurrentTickClosure;

		//my client data
		ClientData m_ClientData;
		//map of other clients data based on their ID's
		std::unordered_map<uint64_t, ClientData> m_ClientMap;

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
		enum BuildMode
		{Normal, Dynamic, Kinematic};
		BuildMode m_BuildMode;
		BrushType m_BrushType = BrushType::circle;
		Element m_HoveredElement = Element();
		bool m_BuildingRigidBody = false;

		glm::ivec2 m_RigidMin = { 99999999, 99999999 };
		glm::ivec2 m_RigidMax = { -99999999 , -99999999 };

		//testing / game
		bool m_SimulationRunning = false;
		bool m_Hovering = false;
		float m_DouglasThreshold = 1.0f;

		//debugging
		int d_LastRecievedInputTick = 0;
	};
}
