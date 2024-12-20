#pragma once

#include "GameLayer.h"
#include "SingleplayerGameLayer.h"
#include "MultiplayerGameLayer.h"
#include "HostingGameLayer.h"
#include "steam/isteamfriends.h"



namespace Pyxis
{
	class MenuLayer : public Layer
	{
	public:
		MenuLayer();
		virtual ~MenuLayer();

		//Layer functions
		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate(Timestep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event& e) override;

		void FailedToConnect();

		void OnGameLayerStarted(GameLayer& layer);
		//bool OnWindowResizeEvent(WindowResizeEvent& event);
		//bool OnKeyPressedEvent(KeyPressedEvent& event);
		//bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		//bool OnMouseScrolledEvent(MouseScrolledEvent& event);

		//std::pair<float, float> GetMousePositionScene();

		

	private:

		//////////////////////////////////////
		/// Steam Callbacks
		//////////////////////////////////////
		//STEAM_CALLBACK(MultiplayerGameLayer, OnGameRichPresenceJoinRequested, GameRichPresenceJoinRequested_t);
		//void OnGameRichPresenceJoinRequested(GameRichPresenceJoinRequested_t* pCallback);
		//void OnGameOverlayActivated(GameOverlayActivated_t* pCallback);

		STEAM_CALLBACK(MenuLayer, OnGameRichPresenceJoinRequested, GameRichPresenceJoinRequested_t, m_CallbackRichPresenceJoinRequested);
		STEAM_CALLBACK(MenuLayer, OnGameOverlayActivated, GameOverlayActivated_t, m_CallbackGameOverlayActivated);

		//CCallback<MenuLayer, GameRichPresenceJoinRequested_t> m_CallbackRichPresenceJoinRequested;
		//CCallback<MenuLayer, GameOverlayActivated_t> m_CallbackGameOverlayActivated;


		std::weak_ptr<SingleplayerGameLayer> m_SinglePlayerLayer;
		std::weak_ptr<MultiplayerGameLayer> m_MultiplayerLayer;
		std::weak_ptr<HostingGameLayer> m_HostingLayer;

		bool m_AnyGameLayerAttached = false;

		float m_PlayerColor[3] = {0,0,0};
		char m_PlayerName[64] = "name";

		//UI things
		bool m_InMainMenu = true;
		char m_InputAddress[22] = "127.0.0.1:21218";

	};
}
