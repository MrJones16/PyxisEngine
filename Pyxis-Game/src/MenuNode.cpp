#include "MenuNode.h"
#include <steam/steam_api.h>

#include <Pyxis/Nodes/UI.h>

#include "SingleplayerGameNode.h"
#include "MultiplayerGameNode.h"
#include "HostedGameNode.h"

namespace Pyxis
{
	MenuNode::MenuNode(const std::string& name) : Node(name),
		m_CallbackRichPresenceJoinRequested(this, &MenuNode::OnGameRichPresenceJoinRequested),
		m_CallbackGameOverlayActivated(this, &MenuNode::OnGameOverlayActivated),
		m_PlayButtonReciever(this, &MenuNode::PlaySinglePlayer)
		//m_WindowResizeReciever(this, &MenuNode::OnWindowResize)
	{
		//since I don't have scenes to set up an actual heirarchy, and an editor to do things
		//scenes are set in a constructor...

		//EventSignal::s_WindowResizeEventSignal.AddReciever(m_WindowResizeReciever);

		FontLibrary::AddFont("Aseprite", "assets/fonts/Aseprite.ttf");

		auto camera = CreateRef<CameraNode>();
		camera->SetMainCamera();
		camera->SetWidth(12.8);
		camera->m_LockAspect = false;
		AddChild(camera);

		auto screenNode = CreateRef<UI::UIScreenSpace>();
		AddChild(screenNode);

		auto canvasNode = CreateRef<UI::UICanvas>();
		canvasNode->CreateTextures("assets/textures/UI/GreenCanvas/", "GreenCanvasTile_", ".png");
		canvasNode->m_PPU = 32;
		canvasNode->m_TextureScale = 32;
		screenNode->AddChild(canvasNode);
		canvasNode->m_AutomaticSizing = true;
		canvasNode->m_AutomaticSizingOffset;

		

		//container
		auto container = CreateRef<UI::Container>();
		container->Translate({ 0,0,-0.001 });
		container->m_AutomaticSizing = true;
		container->m_AutomaticSizingOffset = { -64, -64 };
		container->m_Direction = UI::Down;
		container->m_HorizontalAlignment = UI::Center;
		container->m_Gap = 8;
		container->m_VerticalAlignment = UI::Up;
		canvasNode->AddChild(container);

		auto logo = CreateRef<UI::UIRect>(ResourceSystem::Load<Texture2DResource>("assets/textures/UI/InsetPyxisLogo.png"), "Logo");
		logo->m_PPU = 0.25f;
		logo->UpdateSizeFromTexture();
		container->AddChild(logo);

		

		//Singleplayer button
		auto playButton = CreateRef<UI::UIButton>("Play-Singleplayer-Button", std::bind(&MenuNode::PlaySinglePlayer, this));
		playButton->m_PPU = 0.25f;
		playButton->Translate({ 0,0,1 });
		playButton->m_TextureResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/SingleplayerButtonVertical.png");
		playButton->m_TexturePressedResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/SingleplayerButtonVerticalPressed.png");
		playButton->UpdateSizeFromTexture();
		container->AddChild(playButton);

		//Multiplayer button
		auto multiButton = CreateRef<UI::UIButton>("Play-Multiplayer-Button", std::bind(&MenuNode::PlayMultiplayer, this));
		multiButton->m_PPU = 0.25f;
		multiButton->Translate({ 0,0,1 });
		multiButton->m_TextureResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/MultiplayerButtonVertical.png");
		multiButton->m_TexturePressedResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/MultiplayerButtonVerticalPressed.png");
		multiButton->UpdateSizeFromTexture();
		container->AddChild(multiButton);

		//Host Game button
		auto hostButton = CreateRef<UI::UIButton>("Host-Button", std::bind(&MenuNode::HostGameP2P, this));
		hostButton->m_PPU = 0.25f;
		hostButton->Translate({ 0,0,1 });
		hostButton->m_TextureResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/HostGameButtonVertical.png");
		hostButton->m_TexturePressedResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/HostGameButtonVerticalPressed.png");
		hostButton->UpdateSizeFromTexture();
		container->AddChild(hostButton);

		////Host Game IP button
		//auto hostButtonIP = CreateRef<UI::UIButton>("Host-Button-IP", std::bind(&MenuNode::HostGameIP, this));
		//hostButtonIP->m_PPU = 0.25f;
		//hostButtonIP->Translate({ 0,0,1 });
		//hostButtonIP->m_TextureResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/HostGameButtonVertical.png");
		//hostButtonIP->m_TexturePressedResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/HostGameButtonVerticalPressed.png");
		//hostButtonIP->UpdateSizeFromTexture();
		//hostButtonIP->m_Color = { 0.8,0.8,0.8,1 };
		//container->AddChild(hostButtonIP);

		//Quit Button
		auto quitButton = CreateRef<UI::UIButton>("QuitButton", std::bind(&MenuNode::QuitGame, this));
		quitButton->m_PPU = 0.25f;
		quitButton->Translate({ 0,0,1 });
		quitButton->m_TextureResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/QuitButton.png");
		quitButton->m_TexturePressedResource = ResourceSystem::Load<Texture2DResource>("assets/textures/UI/QuitButtonPressed.png");
		quitButton->UpdateSizeFromTexture();
		container->AddChild(quitButton);


		//manually calling the propagate update so the container and canvas can do their thing :)
		screenNode->PropagateUpdate();
	}

	MenuNode::~MenuNode()
	{

	}


	void MenuNode::OnUpdate(Timestep ts)
	{
		SteamAPI_RunCallbacks();
	}


	void MenuNode::PlaySinglePlayer()
	{
		
		auto spGame = CreateRef<SinglePlayerGameNode>();
		spGame->Start();
		m_Parent->AddChild(spGame);
		QueueFree();
	}

	void MenuNode::PlayMultiplayer()
	{
		/*PX_WARN("Pressed Multiplayer!");
		PX_WARN("Connecting to 127.0.0.1:21218");
		auto MultiplayerGame = CreateRef<MultiplayerGameNode>();
		MultiplayerGame->Connect("127.0.0.1:21218");
		m_Parent->AddChild(MultiplayerGame);
		QueueFree();*/

		SteamFriends()->ActivateGameOverlay("friends");
	}

	void MenuNode::HostGameP2P()
	{
		
		auto HostGame = CreateRef<HostedGameNode>();
		HostGame->StartP2P();
		m_Parent->AddChild(HostGame);
		QueueFree();
	}

	void MenuNode::HostGameIP()
	{
		PX_WARN("Host!");
		auto HostGame = CreateRef<HostedGameNode>();
		HostGame->StartIP();
		m_Parent->AddChild(HostGame);
		QueueFree();
	}

	void MenuNode::QuitGame()
	{
		Application::Get().Close();
	}

	/*void MenuNode::OnWindowResize(WindowResizeEvent& event)
	{
		m_CanvasNode->m_Size = { Camera::s_MainCamera->GetWidth(), Camera::s_MainCamera->GetHeight() };
		m_CanvasNode->UpdateCanvasTransforms();
		if (auto container = dynamic_cast<UI::Container*>(m_CanvasNode->m_Children.front().get()))
		{
			container->m_Size = m_CanvasNode->m_Size - glm::vec2(1);
			container->RearrangeChildren();
		}
	}*/

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