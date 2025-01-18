#pragma once

#include "Pyxis.h"
#include "Pyxis/Core/Panel.h"
#include "Pyxis/Core/ProfilingPanel.h"
#include <Pyxis/Network/NetworkClient.h>
#include <Pyxis/Network/NetworkServer.h>

#include "World.h"
#include <Pyxis/Events/EventSignals.h>

#include <Pyxis/Nodes/OrthographicCameraControllerNode.h>
#include <Pyxis/Nodes/UI.h>



namespace Pyxis
{

	class GameNode : public Node
	{
	public:
		GameNode(std::string debugName = "Game Layer");
		virtual ~GameNode();

		//game node can force hold a camera since it is very important to it
		Ref<OrthographicCameraControllerNode> m_CameraController;


		//////////////////////////////////////
		/// Game Engine Event Recievers & Functions
		//////////////////////////////////////

		Reciever<void(KeyPressedEvent&)> m_KeyPressedReciever;
		void OnKeyPressedEvent(KeyPressedEvent& event);

		Reciever<void(MouseButtonPressedEvent&)> m_MouseButtonPressedReciever;
		void OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);

		Reciever<void(MouseScrolledEvent&)> m_MouseScrolledReciever;
		void OnMouseScrolledEvent(MouseScrolledEvent& event);

		/*Reciever<void(WindowResizeEvent&)> m_WindowResizeReciever;
		void OnWindowResizeEvent(WindowResizeEvent& event);*/


		//UI
		Ref<UI::UICanvas> m_Hotbar;

		Ref<UI::UIButton> m_PlayButton;
		Ref<UI::UIButton> m_PauseButton;
		void Play() { m_PlayButton->m_Enabled = false; m_PauseButton->m_Enabled = true; m_World.m_Running = true; PX_TRACE("Play!"); };
		void Pause() { m_PlayButton->m_Enabled = true; m_PauseButton->m_Enabled = false; m_World.m_Running = false; PX_TRACE("Pause!"); };

		void SetBrushType(BrushType type)
		{
			m_BrushType = type;
		}

		void SetBrushElement(int ID)
		{
			m_SelectedElementIndex = ID;
			if (m_SelectedElementIndex < 0) m_SelectedElementIndex = 0;
			if (m_SelectedElementIndex > m_World.m_ElementData.size()) m_SelectedElementIndex = m_World.m_ElementData.size();
		}

		//////////////////////////////////////
		/// Game Functions
		//////////////////////////////////////
		void GameUpdate(Timestep ts);
		virtual void ClientImGuiRender();
		void HandleTickClosure(MergedTickClosure& tc);
		
		
		void PaintBrushHologram();
		void TextCentered(std::string text);
		glm::ivec2 GetMousePositionImGui();
		glm::vec2 GetMousePosWorld();

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

		glm::vec2 m_ViewportBounds[2];

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
		World m_World;
		

		bool m_Hovering = false;

		//the current input tick we are at
		//starts at -1 or "max value", so when connecting, if we recieve mtc's from 
		//before the world data, we discard them.
		uint64_t m_InputTick = -1;		

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
