#include "MenuNode.h"
#include <steam/steam_api.h>

namespace Pyxis
{
	MenuNode::MenuNode(const std::string& name) : Node(name),
		m_CallbackRichPresenceJoinRequested(this, &MenuNode::OnGameRichPresenceJoinRequested),
		m_CallbackGameOverlayActivated(this, &MenuNode::OnGameOverlayActivated),
		m_PlayButtonReciever(this, &MenuNode::PlayButton)
	{
		//since I don't have scenes to set up an actual heirarchy, and an editor to do things
		//scenes are set in a constructor...

		FontLibrary::AddFont("Aseprite", "assets/fonts/Aseprite.ttf");

		auto canvas = CreateRef<UI::UICanvas>();
		canvas->CreateTextures("assets/textures/GreenCanvas/", "GreenCanvasTile_", ".png");
		Camera* camera = Camera::Main();
		if (camera)
		{
			//base canvas
			canvas->m_Size = { camera->GetWidth(),camera->GetHeight()};
			AddChild(canvas);

			//container
			auto container = CreateRef<UI::UIContainer>();
			container->Translate({ 0,0,1 });
			canvas->AddChild(container);
			container->m_Size = canvas->m_Size - glm::vec2(1);
			container->m_Direction = UI::UIContainer::Down;
			container->m_Color = { 0,0,0,0 };

			//play button!
			auto playButton = CreateRef<UI::UIButton>();
			playButton->Translate({ 0,0,1 });
			playButton->m_Size = { 3, 1 };
			playButton->m_Color = { 0.1f, 0.8f, 0.1f, 1.0f };
			playButton->m_Texture = Texture2D::Create("assets/textures/UIButton.png");
			playButton->AddReciever(m_PlayButtonReciever);

			//add child after setting the dimensions, because otherwise ArrangeChildren isn't called
			//in the container
			container->AddChild(playButton);
		}
	}

	MenuNode::~MenuNode()
	{

	}



	void MenuNode::OnUpdate(Timestep ts)
	{
		/*if (m_SinglePlayerLayer.expired() && m_MultiplayerLayer.expired() && m_HostingLayer.expired())
		{
			m_AnyGameLayerAttached = false;
		}*/
		SteamAPI_RunCallbacks();
	}

	

	//void MenuNode::OnImGuiRender()
	//{
	//	if (!m_AnyGameLayerAttached)
	//	{
	//		auto dock = ImGui::DockSpaceOverViewport(ImGui::GetID("MenuDock"), (const ImGuiViewport*)0, ImGuiDockNodeFlags_PassthruCentralNode);
	//		//show main menu
	//		ImGui::SetNextWindowDockID(dock);
	//		if (ImGui::Begin("Main Menu", (bool*)0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar))
	//		{
	//			ImGui::Text("Pyxis");
	//	

	//			if (ImGui::Button("Start Singleplayer"))
	//			{
	//				Ref<SingleplayerGameLayer> ref = CreateRef<SingleplayerGameLayer>();
	//				OnGameLayerStarted(*ref);
	//				ref->Start();
	//				
	//				Application::Get().PushLayer(ref);
	//				m_SinglePlayerLayer = ref;
	//				m_AnyGameLayerAttached = true;
	//			}

	//			ImGui::InputText("IP Address", m_InputAddress, 22);
	//			if (ImGui::Button("Connect"))
	//			{
	//				Ref<MultiplayerGameLayer> ref = CreateRef<MultiplayerGameLayer>();
	//				OnGameLayerStarted(*ref);
	//				ref->ConnectIP(std::string(m_InputAddress));
	//				Application::Get().PushLayer(ref);
	//				m_MultiplayerLayer = ref;
	//				m_AnyGameLayerAttached = true;
	//			}

	//			if (ImGui::Button("Host over Steam"))
	//			{
	//				Ref<HostingGameLayer> ref = CreateRef<HostingGameLayer>();
	//				OnGameLayerStarted(*ref);
	//				ref->StartP2P(0);
	//				Application::Get().PushLayer(ref);
	//				m_HostingLayer = ref;
	//				m_AnyGameLayerAttached = true;
	//			}

	//			if (ImGui::Button("Host by IP"))
	//			{
	//				Ref<HostingGameLayer> ref = CreateRef<HostingGameLayer>();
	//				OnGameLayerStarted(*ref);
	//				ref->StartIP();
	//				Application::Get().PushLayer(ref);
	//				m_HostingLayer = ref;
	//				m_AnyGameLayerAttached = true;
	//			}
	//			
	//			if (ImGui::BeginChild("Player Customization", { 0,0 }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY))
	//			{
	//				ImGui::Text("Player Customization");
	//				//ImGui::ColorPicker3("Player Color", m_PlayerColor);
	//				ImGui::ColorEdit3("Player Color", m_PlayerColor);
	//				ImGui::InputText("Player Name", m_PlayerName, 64);

	//				ImGui::EndChild();
	//			}
	//			

	//			if (ImGui::Button("Quit Game"))
	//			{
	//				Application::Get().Close();
	//			}

	//		}
	//		ImGui::End();
	//		
	//	}
	//	
	//}

	//void MenuNode::OnEvent(Event& e)
	//{

	//}

	void MenuNode::PlayButton()
	{
		PX_WARN("Pressed Play!!");
	}

	void MenuNode::FailedToConnect()
	{

	}

	//void MenuNode::OnGameLayerStarted(GameLayer& layer)
	//{
	//	layer.m_ClientData.m_Color = glm::vec4(m_PlayerColor[0], m_PlayerColor[1], m_PlayerColor[2], 1);
	//	memcpy(layer.m_ClientData.m_Name, m_PlayerName, 64);
	//}

	void MenuNode::OnGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* pCallback)
	{
		/////start with killing any active instance
		//Application& app = Application::Get();
		//if (!m_SinglePlayerLayer.expired())
		//{
		//	app.PopLayerQueue(m_SinglePlayerLayer.lock());
		//}
		//if (!m_MultiplayerLayer.expired())
		//{
		//	app.PopLayerQueue(m_MultiplayerLayer.lock());
		//}
		//if (!m_HostingLayer.expired())
		//{
		//	app.PopLayerQueue(m_HostingLayer.lock());
		//}

		////create a multiplayer instance and connect
		//SteamNetworkingIdentity identity;
		//identity.SetSteamID(pCallback->m_steamIDFriend);
		//Ref<MultiplayerGameLayer> ref = CreateRef<MultiplayerGameLayer>();
		//ref->Connect(identity);
		//Application::Get().PushLayer(ref);
		//m_MultiplayerLayer = ref;
		//m_AnyGameLayerAttached = true;

	}
	void MenuNode::OnGameOverlayActivated(GameOverlayActivated_t* pCallback)
	{
		PX_WARN("Successfully detected opening of overlay!");
	}
}