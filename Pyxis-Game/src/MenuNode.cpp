#include "MenuNode.h"
#include <steam/steam_api.h>

#include <Pyxis/Nodes/UI/UI.h>

#include "SingleplayerGameNode.h"
#include "MultiplayerGameNode.h"
#include "HostedGameNode.h"
#include <Pyxis/Nodes/PixelCameraNode.h>

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
		glm::vec4 themeYellow = glm::vec4(255.0f / 255.0f, 221.0f / 255.0f, 159.0f / 255.0f, 1);

		auto camera = Instantiate<PixelCameraNode>();
		camera->SetMainCamera();
		camera->SetWidth(640);
		camera->m_LockAspect = false;
		AddChild(camera);

		auto screenNode = Instantiate<UI::ScreenSpace>();
		AddChild(screenNode);

		auto canvasNode = Instantiate<UI::Canvas>();
		canvasNode->CreateTextures("assets/textures/UI/GreenCanvas/", "GreenCanvasTile_", ".png");
		canvasNode->m_PPU = 32;
		screenNode->AddChild(canvasNode);
		canvasNode->m_AutomaticSizing = true;
		canvasNode->m_AutomaticSizingOffset;


		//container
		auto container = Instantiate<UI::Container>();
		container->Translate({ 0,0,-0.001 });
		container->m_AutomaticSizing = true;
		container->m_AutomaticSizingOffset = { -64, -64 };
		container->m_Direction = UI::Down;
		container->m_HorizontalAlignment = UI::Center;
		container->m_Gap = 8;
		container->m_VerticalAlignment = UI::Up;
		canvasNode->AddChild(container);


		//logo
		auto logo = Instantiate<UI::UIRect>(ResourceManager::Load<Texture2DResource>("assets/textures/UI/InsetPyxisLogo.png"), "Logo");
		logo->m_PPU = 0.25f;
		logo->UpdateSizeFromTexture();
		container->AddChild(logo);


		//singleplayer button
		{
			auto playButton = Instantiate<UI::TextButton>("Singleplayer Button", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&MenuNode::PlaySinglePlayer, this));
			playButton->m_PPU = 0.25f;
			playButton->m_Text = "Singleplayer";
			playButton->m_TextColor = themeYellow;
			playButton->Translate({ 0,0,1 });
			playButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
			playButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
			playButton->UpdateSizeFromTexture();
			playButton->m_TextBorderSize = glm::vec2(5, 5);
			playButton->m_TextOffset = { 0, 3, -0.0001f };
			playButton->m_TextOffsetPressed = { 0, 1, -0.0001f };
			container->AddChild(playButton);
		}


		//client data input
		{
			//name
			auto nameInput = Instantiate<UI::InputText>("Name Text Input", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), &m_PlayerName);
			nameInput->m_FontSize = 5;
			nameInput->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlate.png");
			nameInput->m_TextureResourceSelected = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateSelected.png");
			nameInput->m_PPU = 0.25;
			nameInput->m_TextBorderSize = glm::vec2(3);
			nameInput->m_Alignment = UI::Direction::Center;
			nameInput->UpdateSizeFromTexture();
			container->AddChild(nameInput);


			////color select canvas
			//auto colorCanvas = CreateRef<UI::Canvas>();
			//colorCanvas->CreateTextures("assets/textures/UI/GreenCanvas/", "GreenCanvasTile_", ".png");
			//colorCanvas->m_PPU = 64;
			//colorCanvas->m_Size = { 512, 128 };
			//container->AddChild(colorCanvas);

			//color select container
			auto colorSelectContainer = Instantiate<UI::Container>("Color Select Container");
			colorSelectContainer->m_VerticalAlignment = UI::Direction::Center;
	/*		colorSelectContainer->m_AutomaticSizing = true;
			colorSelectContainer->m_AutomaticSizingOffset = { -48,-48 };*/
			colorSelectContainer->m_Size = { 470, 64 };
			colorSelectContainer->Translate({ 0, 0, -0.01 });
			colorSelectContainer->m_Gap = 4;
			container->AddChild(colorSelectContainer);

			auto colorDisplay = Instantiate<UI::UIRect>("Color Rect");
			m_PlayerColorDisplay = colorDisplay; // grab weak ref
			colorDisplay->m_Size = { 64, 64 };

			auto colorRedInput = Instantiate<UI::InputInt>("Color Red Input", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), &m_PlayerColor[0]); // &red color
			colorRedInput->m_MaxValue = 255;
			colorRedInput->m_MinValue = 0;
			colorRedInput->m_FontSize = 5;
			colorRedInput->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateSmall.png");
			colorRedInput->m_TextureResourceSelected = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateSmallSelected.png");
			colorRedInput->m_PPU = 0.25;
			colorRedInput->m_TextBorderSize = glm::vec2(3);
			colorRedInput->m_Alignment = UI::Direction::Center;
			colorRedInput->UpdateSizeFromTexture();
			colorSelectContainer->AddChild(colorRedInput);

			auto colorGreenInput = Instantiate<UI::InputInt>("Color Green Input", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), &m_PlayerColor[1]); // &Green color
			colorGreenInput->m_MaxValue = 255;
			colorGreenInput->m_MinValue = 0;
			colorGreenInput->m_FontSize = 5;
			colorGreenInput->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateSmall.png");
			colorGreenInput->m_TextureResourceSelected = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateSmallSelected.png");
			colorGreenInput->m_PPU = 0.25;
			colorGreenInput->m_TextBorderSize = glm::vec2(3);
			colorGreenInput->m_Alignment = UI::Direction::Center;
			colorGreenInput->UpdateSizeFromTexture();
			colorSelectContainer->AddChild(colorGreenInput);

			auto colorBlueInput = Instantiate<UI::InputInt>("Color Blue Input", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), &m_PlayerColor[2]); // &Blue color
			colorBlueInput->m_MaxValue = 255;
			colorBlueInput->m_MinValue = 0;
			colorBlueInput->m_FontSize = 5;
			colorBlueInput->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateSmall.png");
			colorBlueInput->m_TextureResourceSelected = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateSmallSelected.png");
			colorBlueInput->m_PPU = 0.25;
			colorBlueInput->m_TextBorderSize = glm::vec2(3);
			colorBlueInput->m_Alignment = UI::Direction::Center;
			colorBlueInput->UpdateSizeFromTexture();
			colorSelectContainer->AddChild(colorBlueInput);

			
			colorSelectContainer->AddChild(colorDisplay);

		}
		

		//multiplayer button
		{
			auto multiButton = Instantiate<UI::TextButton>("Join Friends Button", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&MenuNode::PlayMultiplayer, this));
			multiButton->m_PPU = 0.25f;
			multiButton->m_Text = "Join Friend";
			multiButton->m_TextColor = themeYellow;
			multiButton->Translate({ 0,0,1 });
			multiButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
			multiButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
			multiButton->UpdateSizeFromTexture();
			multiButton->m_TextBorderSize = glm::vec2(5, 5);
			multiButton->m_TextOffset = { 0, 3, -0.0001f };
			multiButton->m_TextOffsetPressed = { 0, 1, -0.0001f };
			container->AddChild(multiButton);
		}

		//LAN Join button
		{
			auto horizContainer = Instantiate<UI::HorizontalContainer>("Connect&IP HContainer");
			horizContainer->m_Arrangement = UI::Right;
			horizContainer->m_Gap = 8;
			container->AddChild(horizContainer);

			auto multiButton = Instantiate<UI::TextButton>("Join LAN Button", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&MenuNode::PlayLAN, this));
			multiButton->m_PPU = 0.25f;
			multiButton->m_Text = "Connect";
			multiButton->m_TextColor = themeYellow;
			multiButton->Translate({ 0,0,1 });
			multiButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
			multiButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
			multiButton->UpdateSizeFromTexture();
			multiButton->m_TextBorderSize = glm::vec2(5, 5);
			multiButton->m_TextOffset = { 0, 3, -0.0001f };
			multiButton->m_TextOffsetPressed = { 0, 1, -0.0001f };
			horizContainer->AddChild(multiButton);

			//IP Input
			auto IPInput = Instantiate<UI::InputText>("IP Input", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), &m_InputAddress);
			IPInput->m_FontSize = 5;
			IPInput->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlate.png");
			IPInput->m_TextureResourceSelected = ResourceManager::Load<Texture2DResource>("assets/textures/UI/TextPlateSelected.png");
			IPInput->m_PPU = 0.25;
			IPInput->m_TextBorderSize = glm::vec2(3);
			IPInput->m_Alignment = UI::Direction::Center;
			IPInput->UpdateSizeFromTexture();
			horizContainer->AddChild(IPInput);
			horizContainer->SetSizeFromChildren();
		}


		//Host Game button
		{
			auto hostButton = Instantiate<UI::TextButton>("Host Game Button", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&MenuNode::HostGameP2P, this));
			hostButton->m_PPU = 0.25f;
			hostButton->m_Text = "Host Game";
			hostButton->m_TextColor = themeYellow;
			hostButton->Translate({ 0,0,1 });
			hostButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
			hostButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
			hostButton->UpdateSizeFromTexture();
			hostButton->m_TextBorderSize = glm::vec2(5, 5);
			hostButton->m_TextOffset = { 0, 3, -0.0001f };
			hostButton->m_TextOffsetPressed = { 0, 1, -0.0001f };
			container->AddChild(hostButton);
		}

		//Host LAN Game button
		{
			auto hostButton = Instantiate<UI::TextButton>("Host LAN Button", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&MenuNode::HostGameIP, this));
			hostButton->m_PPU = 0.25f;
			hostButton->m_Text = "Host LAN";
			hostButton->m_TextColor = themeYellow;
			hostButton->Translate({ 0,0,1 });
			hostButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
			hostButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
			hostButton->UpdateSizeFromTexture();
			hostButton->m_TextBorderSize = glm::vec2(5, 5);
			hostButton->m_TextOffset = { 0, 3, -0.0001f };
			hostButton->m_TextOffsetPressed = { 0, 1, -0.0001f };
			container->AddChild(hostButton);
		}


		//Quit Button	
		{
				
			auto quitButton = Instantiate<UI::TextButton>("Quit Game Button", ResourceManager::Load<Font>("assets/fonts/Aseprite.ttf"), std::bind(&MenuNode::QuitGame, this));
			quitButton->m_PPU = 0.25f;
			quitButton->m_Text = "Quit Game";
			quitButton->m_TextColor = themeYellow;
			quitButton->Translate({ 0,0,1 });
			quitButton->m_TextureResource = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWide.png");
			quitButton->m_TextureResourcePressed = ResourceManager::Load<Texture2DResource>("assets/textures/UI/ButtonWidePressed.png");
			quitButton->UpdateSizeFromTexture();
			quitButton->m_TextBorderSize = glm::vec2(5, 5);
			quitButton->m_TextOffset = { 0, 3, -0.0001f };
			quitButton->m_TextOffsetPressed = { 0, 1, -0.0001f };
			container->AddChild(quitButton);
		}


		//manually calling the propagate update so the container and canvas can do their thing :)
		screenNode->PropagateUpdate();
	}

	MenuNode::~MenuNode()
	{

	}


	void MenuNode::OnUpdate(Timestep ts)
	{
		if (auto colorDisplay = m_PlayerColorDisplay.lock())
		{
			colorDisplay->m_Color = glm::vec4((float)m_PlayerColor[0] / 255.0f, (float)m_PlayerColor[1] / 255.0f, (float)m_PlayerColor[2] / 255.0f, (float)m_PlayerColor[3] / 255.0f);
		}
	}


	void MenuNode::PlaySinglePlayer()
	{
		
		auto spGame = Instantiate<SinglePlayerGameNode>();
		spGame->Start();
		QueueFreeHierarchy();
	}

	void MenuNode::PlayMultiplayer()
	{
		SteamFriends()->ActivateGameOverlay("friends");
	}

	void MenuNode::PlayLAN()
	{
		//Get steam identity, create a multiplayer instance, and connect, and kill menu node.
		auto MultiplayerGame = Instantiate<MultiplayerGameNode>();
		MultiplayerGame->m_ClientData.m_Color = glm::vec4((float)m_PlayerColor[0] / 255.0f, (float)m_PlayerColor[1] / 255.0f, (float)m_PlayerColor[2] / 255.0f, (float)m_PlayerColor[3] / 255.0f);
		std::memcpy(MultiplayerGame->m_ClientData.m_Name, m_PlayerName.c_str(), 64);
		MultiplayerGame->Connect("127.0.0.1:21218");
		QueueFreeHierarchy();
	}

	void MenuNode::HostGameP2P()
	{
		
		auto HostGame = Instantiate<HostedGameNode>();
		HostGame->StartP2P();
		QueueFreeHierarchy();
	}

	void MenuNode::HostGameIP()
	{
		
		auto HostGame = Instantiate<HostedGameNode>();
		HostGame->StartIP();
		QueueFreeHierarchy();
	}

	void MenuNode::QuitGame()
	{
		Application::Get().Close();
	}


	void MenuNode::FailedToConnect()
	{

	}

	void MenuNode::OnGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* pCallback)
	{

		//Get steam identity, create a multiplayer instance, and connect, and kill menu node.
		SteamNetworkingIdentity identity;
		identity.SetSteamID(pCallback->m_steamIDFriend);
		auto MultiplayerGame = Instantiate<MultiplayerGameNode>();
		MultiplayerGame->m_ClientData.m_Color = glm::vec4((float)m_PlayerColor[0] / 255.0f, (float)m_PlayerColor[1] / 255.0f, (float)m_PlayerColor[2] / 255.0f, (float)m_PlayerColor[3] / 255.0f);
		std::memcpy(MultiplayerGame->m_ClientData.m_Name, m_PlayerName.c_str(), 64);
		MultiplayerGame->Connect(identity);				
		QueueFreeHierarchy();
	}
	void MenuNode::OnGameOverlayActivated(GameOverlayActivated_t* pCallback)
	{
		PX_WARN("Successfully detected opening of overlay!");
	}
}