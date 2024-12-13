#pragma once

#include "GameLayer.h"
#include "SingleplayerGameLayer.h"
#include "MultiplayerGameLayer.h"
#include "HostingGameLayer.h"


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
		//bool OnWindowResizeEvent(WindowResizeEvent& event);
		//bool OnKeyPressedEvent(KeyPressedEvent& event);
		//bool OnMouseButtonPressedEvent(MouseButtonPressedEvent& event);
		//bool OnMouseScrolledEvent(MouseScrolledEvent& event);

		//std::pair<float, float> GetMousePositionScene();

	private:

		std::weak_ptr<SingleplayerGameLayer> m_SinglePlayerLayer;
		std::weak_ptr<MultiplayerGameLayer> m_MultiplayerLayer;
		std::weak_ptr<HostingGameLayer> m_HostingLayer;

		bool m_AnyGameLayerAttached = false;

		//UI things
		bool m_InMainMenu = true;
		char m_InputAddress[22] = "127.0.0.1:21218";

	};
}
