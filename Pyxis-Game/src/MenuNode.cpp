#include "MenuNode.h"
#include <steam/steam_api.h>

#include "SingleplayerGameNode.h"

namespace Pyxis
{
	MenuNode::MenuNode(const std::string& name) : Node(name),
		m_CallbackRichPresenceJoinRequested(this, &MenuNode::OnGameRichPresenceJoinRequested),
		m_CallbackGameOverlayActivated(this, &MenuNode::OnGameOverlayActivated),
		m_PlayButtonReciever(this, &MenuNode::PlaySinglePlayer),
		m_WindowResizeReciever(this, &MenuNode::OnWindowResize)
	{
		//since I don't have scenes to set up an actual heirarchy, and an editor to do things
		//scenes are set in a constructor...

		EventSignal::s_WindowResizeEventSignal.AddReciever(m_WindowResizeReciever);

		FontLibrary::AddFont("Aseprite", "assets/fonts/Aseprite.ttf");

		m_CanvasNode = CreateRef<UI::UICanvas>();
		m_CanvasNode->CreateTextures("assets/textures/UI/GreenCanvas/", "GreenCanvasTile_", ".png");
		auto camera = CreateRef<CameraNode>();
		camera->SetWidth(12.8);
		AddChild(camera);
		
		//base canvas
		m_CanvasNode->m_Size = { camera->GetWidth(),camera->GetHeight()};
		m_CanvasNode->UpdateCanvasTransforms();
		AddChild(m_CanvasNode);

		//container
		auto container = CreateRef<UI::Container>();
		container->Translate({ 0,0,1 });
		m_CanvasNode->AddChild(container);
		container->m_Size = m_CanvasNode->m_Size - glm::vec2(1);
		container->m_Direction = UI::Down;
		container->m_HorizontalAlignment = UI::Center;
		container->m_VerticalAlignment = UI::Up;
		container->m_Color = { 0,0,0,0 };

		auto logo = CreateRef<UI::UIRect>(Texture2D::Create("assets/textures/UI/InsetPyxisLogo.png"), "Logo");
		logo->m_Size = { ((float)logo->m_Texture->GetWidth() / 32.0f), ((float)logo->m_Texture->GetHeight() / 32.0f) };
		container->AddChild(logo);

		//add child after setting the dimensions, because otherwise ArrangeChildren isn't called
		//in the container

		//Singleplayer button
		auto playButton = CreateRef<UI::UIButton>("Play-Singleplayer-Button", std::bind(&MenuNode::PlaySinglePlayer, this));
		playButton->Translate({ 0,0,1 });
		playButton->m_Texture = Texture2D::Create("assets/textures/UI/SingleplayerButtonVertical.png");
		playButton->m_TexturePressed = Texture2D::Create("assets/textures/UI/SingleplayerButtonVerticalPressed.png");
		playButton->UpdateSizeFromTexture();
		container->AddChild(playButton);

		//Multiplayer button
		auto multiButton = CreateRef<UI::UIButton>("Play-Multiplayer-Button", std::bind(&MenuNode::PlayMultiplayer, this));
		multiButton->Translate({ 0,0,1 });
		multiButton->m_Texture = Texture2D::Create("assets/textures/UI/MultiplayerButtonVertical.png");
		multiButton->m_TexturePressed = Texture2D::Create("assets/textures/UI/MultiplayerButtonVerticalPressed.png");
		multiButton->UpdateSizeFromTexture();
		container->AddChild(multiButton);

		//Host Game button
		auto hostButton = CreateRef<UI::UIButton>("Host-Button", std::bind(&MenuNode::HostGame, this));
		hostButton->Translate({ 0,0,1 });
		hostButton->m_Texture = Texture2D::Create("assets/textures/UI/HostGameButtonVertical.png");
		hostButton->m_TexturePressed = Texture2D::Create("assets/textures/UI/HostGameButtonVerticalPressed.png");
		hostButton->UpdateSizeFromTexture();
		container->AddChild(hostButton);

		
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

	void MenuNode::PlaySinglePlayer()
	{
		PX_WARN("Pressed Play!!");
		auto spGame = CreateRef<SinglePlayerGameNode>();
		spGame->Start();
		m_Parent->AddChild(spGame);
		QueueFree();
	}

	void MenuNode::PlayMultiplayer()
	{
		PX_WARN("Multiplayer!");
	}

	void MenuNode::HostGame()
	{
		PX_WARN("Host!");
	}

	void MenuNode::OnWindowResize(WindowResizeEvent& event)
	{
		m_CanvasNode->m_Size = { Camera::s_MainCamera->GetWidth(), Camera::s_MainCamera->GetHeight() };
		m_CanvasNode->UpdateCanvasTransforms();
		if (auto container = dynamic_cast<UI::Container*>(m_CanvasNode->m_Children.front().get()))
		{
			container->m_Size = m_CanvasNode->m_Size - glm::vec2(1);
			container->RearrangeChildren();
		}
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