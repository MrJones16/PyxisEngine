#pragma once

#include "Pyxis.h"
#include "Pyxis/Game/Scene.h"
#include "Pyxis/Core/OrthographicCameraController.h"
#include "Pyxis/Core/Panel.h"
#include "Pyxis/Core/ProfilingPanel.h"
#include "Pyxis/Game/InspectorPanel.h"
#include "Pyxis/Game/SceneHierarchyPanel.h"
#include <Pyxis/Network/NetworkClient.h>
#include <Pyxis/Network/NetworkServer.h>

#include "World.h"



namespace Pyxis
{

	class GameLayer : public Layer
	{
	public:
		GameLayer(std::string debugName = "Game Layer");
		virtual ~GameLayer();

		//////////////////////////////////////
		/// Layer Functions
		//////////////////////////////////////
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnEvent(Event& e) override;


		//////////////////////////////////////
		//////////////////////////////////////
		/// Game Engine Events
		//////////////////////////////////////
		bool OnWindowResizeEvent(WindowResizeEvent& event);
		bool OnKeyPressedEvent(KeyPressedEvent& event);
		bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		bool OnMouseScrolledEvent(MouseScrolledEvent& event);


		//////////////////////////////////////
		/// Game Functions
		//////////////////////////////////////
		void GameUpdate(Timestep ts);
		virtual void ClientImGuiRender(ImGuiID dockID);
		void HandleTickClosure(MergedTickClosure& tc);
		bool CreateWorld();
		void PaintBrushHologram();
		void TextCentered(std::string text);
		std::pair<float, float> GetMousePositionScene();

	public:

		struct ClientData
		{
			char m_Name[64] = "PyxisEnjoyer";
			glm::vec2 m_CursorWorldPosition = { 0,0 };
			glm::vec4 m_Color = { 1,1,1,1 };

			ClientData()
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
				m_Color = colorOptions[std::rand() % colorOptions.size()];
			}
		};
		//my client data
		ClientData m_ClientData;

	protected:

		

		//map of other clients data based on their ID's
		std::unordered_map<HSteamNetConnection, ClientData> m_ClientDataMap;

		//accumulation of input to be shared or used
		TickClosure m_CurrentTickClosure;
		
	protected:

		//////////////////////////////////////
		/// Testing / Debugging
		//////////////////////////////////////
		int d_LastRecievedInputTick = 0;
		float m_DouglasThreshold = 1.0f;

		//////////////////////////////////////
		/// Game Variables
		//////////////////////////////////////
		Ref<World> m_World;
		bool m_SimulationRunning = false;
		//time vars for update rate
		std::chrono::time_point<std::chrono::high_resolution_clock> m_UpdateTime = std::chrono::high_resolution_clock::now();
		std::chrono::time_point<std::chrono::high_resolution_clock> m_SlowUpdateTime = std::chrono::high_resolution_clock::now();
		float m_TickRate = 30.0f;
		float m_TickRateSlow = 10.0f;

		//the current input tick we are at
		//starts at -1 or "max value", so when connecting, if we recieve mtc's from 
		//before the world data, we discard them.
		uint64_t m_InputTick = -1;

		//scene things
		Ref<FrameBuffer> m_SceneFrameBuffer;
		Ref<Scene> m_Scene;
		OrthographicCameraController m_OrthographicCameraController;
		glm::vec2 m_ViewportSize;
		ImVec2 m_ViewportOffset;
		bool m_SceneViewIsFocused = false;
		bool m_Hovering = false;

		//Tools / Panels
		std::vector<Ref<Panel>> m_Panels;

		//player tools
		int m_SelectedElementIndex = 0;
		float m_BrushSize = 1;
		enum BuildMode
		{
			Normal, Dynamic, Kinematic
		};
		BuildMode m_BuildMode = BuildMode::Normal;
		BrushType m_BrushType = BrushType::circle;
		Element m_HoveredElement = Element();
		bool m_BuildingRigidBody = false;

		glm::ivec2 m_RigidMin = { 99999999, 99999999 };
		glm::ivec2 m_RigidMax = { -99999999 , -99999999 };
		
	};
}
