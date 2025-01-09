#include "MenuLayer.h"
#include <steam/steam_api.h>

namespace Pyxis
{
	MenuLayer::MenuLayer() :
		m_CallbackRichPresenceJoinRequested(this, &MenuLayer::OnGameRichPresenceJoinRequested),
		m_CallbackGameOverlayActivated(this, &MenuLayer::OnGameOverlayActivated)
	{

	}

	MenuLayer::~MenuLayer()
	{

	}

	void MenuLayer::OnAttach()
	{

	}

	void MenuLayer::OnDetach()
	{

	}


	void MenuLayer::OnUpdate(Timestep ts)
	{
		if (m_SinglePlayerLayer.expired() && m_MultiplayerLayer.expired() && m_HostingLayer.expired())
		{
			m_AnyGameLayerAttached = false;
		}
		SteamAPI_RunCallbacks();
	}

	

	void MenuLayer::OnImGuiRender()
	{
		if (!m_AnyGameLayerAttached)
		{
			auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MenuDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
			//show main menu
			ImGui::SetNextWindowDockID(dock);
			if (ImGui::Begin("Main Menu", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
			{
				ImGui::Text("Pyxis");
		

				if (ImGui::Button("Start Singleplayer"))
				{
					Ref<SingleplayerGameLayer> ref = CreateRef<SingleplayerGameLayer>();
					OnGameLayerStarted(*ref);
					ref->Start();
					
					Application::Get().PushLayer(ref);
					m_SinglePlayerLayer = ref;
					m_AnyGameLayerAttached = true;
				}

				ImGui::InputText("IP Address", m_InputAddress, 22);
				if (ImGui::Button("Connect"))
				{
					Ref<MultiplayerGameLayer> ref = CreateRef<MultiplayerGameLayer>();
					OnGameLayerStarted(*ref);
					ref->ConnectIP(std::string(m_InputAddress));
					Application::Get().PushLayer(ref);
					m_MultiplayerLayer = ref;
					m_AnyGameLayerAttached = true;
				}

				if (ImGui::Button("Host over Steam"))
				{
					Ref<HostingGameLayer> ref = CreateRef<HostingGameLayer>();
					OnGameLayerStarted(*ref);
					ref->StartP2P(0);
					Application::Get().PushLayer(ref);
					m_HostingLayer = ref;
					m_AnyGameLayerAttached = true;
				}

				if (ImGui::Button("Host by IP"))
				{
					Ref<HostingGameLayer> ref = CreateRef<HostingGameLayer>();
					OnGameLayerStarted(*ref);
					ref->StartIP();
					Application::Get().PushLayer(ref);
					m_HostingLayer = ref;
					m_AnyGameLayerAttached = true;
				}
				
				if (ImGui::BeginChild("Player Customization", { 0,0 }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY))
				{
					ImGui::Text("Player Customization");
					//ImGui::ColorPicker3("Player Color", m_PlayerColor);
					ImGui::ColorEdit3("Player Color", m_PlayerColor);
					ImGui::InputText("Player Name", m_PlayerName, 64);

					ImGui::EndChild();
				}
				

				if (ImGui::Button("Quit Game"))
				{
					Application::Get().Close();
				}

			}
			ImGui::End();
			
		}
		
	}

	void MenuLayer::OnEvent(Event& e)
	{

	}

	void MenuLayer::FailedToConnect()
	{

	}

	void MenuLayer::OnGameLayerStarted(GameLayer& layer)
	{
		layer.m_ClientData.m_Color = glm::vec4(m_PlayerColor[0], m_PlayerColor[1], m_PlayerColor[2], 1);
		memcpy(layer.m_ClientData.m_Name, m_PlayerName, 64);
	}

	void MenuLayer::OnGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* pCallback)
	{
		///start with killing any active instance
		Application& app = Application::Get();
		if (!m_SinglePlayerLayer.expired())
		{
			app.PopLayerQueue(m_SinglePlayerLayer.lock());
		}
		if (!m_MultiplayerLayer.expired())
		{
			app.PopLayerQueue(m_MultiplayerLayer.lock());
		}
		if (!m_HostingLayer.expired())
		{
			app.PopLayerQueue(m_HostingLayer.lock());
		}

		//create a multiplayer instance and connect
		SteamNetworkingIdentity identity;
		identity.SetSteamID(pCallback->m_steamIDFriend);
		Ref<MultiplayerGameLayer> ref = CreateRef<MultiplayerGameLayer>();
		ref->Connect(identity);
		Application::Get().PushLayer(ref);
		m_MultiplayerLayer = ref;
		m_AnyGameLayerAttached = true;

	}
	void MenuLayer::OnGameOverlayActivated(GameOverlayActivated_t* pCallback)
	{
		PX_WARN("Successfully detected opening of overlay!");
	}
}