#pragma once

#include <Pyxis/Core/Log.h>
#include <Pyxis/Core/Core.h>
#include <Pyxis/Nodes/Node.h>
#include <Pyxis/Nodes/UI/UI.h>
#include <Pyxis/Events/Signal.h>

#include "steam/isteamfriends.h"



namespace Pyxis
{
	
	class MenuNode : public Node
	{
	public:
		MenuNode(const std::string& name = "MenuNode");

		~MenuNode();

		virtual void OnUpdate(Timestep ts) override;

		//reciever functions for buttons & UI
		void PlaySinglePlayer();
		void PlayMultiplayer();
		void HostGameP2P();
		void HostGameIP();

		void QuitGame();

		void FailedToConnect();

	private:

		//recievers
		Reciever<void()> m_PlayButtonReciever;

		//////////////////////////////////////
		/// Steam Callbacks
		//////////////////////////////////////

		STEAM_CALLBACK(MenuNode, OnGameRichPresenceJoinRequested, GameRichPresenceJoinRequested_t, m_CallbackRichPresenceJoinRequested);
		STEAM_CALLBACK(MenuNode, OnGameOverlayActivated, GameOverlayActivated_t, m_CallbackGameOverlayActivated);

		bool m_AnyGameLayerAttached = false;

		std::weak_ptr<UI::UIRect> m_PlayerColorDisplay;
		int m_PlayerColor[4] = {210,210,210,255};
		char m_PlayerName[64] = "name";

		//UI things
		bool m_InMainMenu = true;
		char m_InputAddress[22] = "127.0.0.1:21218";

	};
}
